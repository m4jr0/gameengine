// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/job/scheduler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/fiber/fiber_life_cycle.h"
#include "comet/core/job/worker_context.h"
#include "comet/core/logger/logging.h"
#include "comet/core/thread/thread_context.h"

namespace comet {
namespace job {
namespace {
void ValidateSchedulerConfig(const SchedulerConfig& config) {
  COMET_ASSERT(config.job_queue_allocator != nullptr,
               "job::ValidateSchedulerConfig", "job queue allocator is null");
  COMET_ASSERT(config.worker_allocator != nullptr,
               "job::ValidateSchedulerConfig", "worker allocator is null");
  COMET_ASSERT(config.fiber_queue_allocator != nullptr,
               "job::ValidateSchedulerConfig", "fiber queue allocator is null");
  COMET_ASSERT(config.fiber_object_allocator != nullptr,
               "job::ValidateSchedulerConfig",
               "fiber object allocator is null");
  COMET_ASSERT(config.fiber_stack_allocator != nullptr,
               "job::ValidateSchedulerConfig", "fiber stack allocator is null");
  COMET_ASSERT(config.fiber_life_cycle_allocator != nullptr,
               "job::ValidateSchedulerConfig",
               "fiber life cycle allocator is null");
  COMET_ASSERT(config.counter_queue_allocator != nullptr,
               "job::ValidateSchedulerConfig",
               "counter queue allocator is null");
  COMET_ASSERT(config.counter_allocator != nullptr,
               "job::ValidateSchedulerConfig", "counter allocator is null");
  COMET_ASSERT(config.main_thread_queue_allocator != nullptr,
               "job::ValidateSchedulerConfig",
               "main thread queue allocator is null");
  COMET_ASSERT(config.main_thread_queue_capacity > 0,
               "job::ValidateSchedulerConfig",
               "main thread queue capacity is zero");
  COMET_ASSERT(config.job_queue_capacity > 0, "job::ValidateSchedulerConfig",
               "job queue capacity is zero");
  COMET_ASSERT(config.counter_count > 0, "job::ValidateSchedulerConfig",
               "counter count is zero");
  COMET_ASSERT(config.large_fiber_count > 0, "job::ValidateSchedulerConfig",
               "large fiber count is zero");
  COMET_ASSERT(config.gigantic_fiber_count > 0, "job::ValidateSchedulerConfig",
               "gigantic fiber count is zero");
#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  COMET_ASSERT(config.external_library_fiber_count > 0,
               "job::ValidateSchedulerConfig",
               "external library fiber count is zero");
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  COMET_ASSERT(config.fiber_life_cycle_queue_capacity > 0,
               "job::ValidateSchedulerConfig",
               "fiber life cycle queue capacity is zero");
  COMET_ASSERT(config.default_io_worker_count > 0,
               "job::ValidateSchedulerConfig",
               "default io worker count is zero");
}
}  // namespace

Scheduler& Scheduler::Get() {
  static Scheduler singleton{};
  return singleton;
}

Scheduler::~Scheduler() {
  COMET_ASSERT(!is_initialized_, "Scheduler::~Scheduler",
               "scheduler is still initialized");
  COMET_ASSERT(config_ == nullptr, "Scheduler::~Scheduler",
               "scheduler config is still attached");
}

void Scheduler::AttachConfig(const SchedulerConfig& config) {
  COMET_ASSERT(!is_initialized_, "Scheduler::AttachConfig",
               "scheduler is already initialized");
  COMET_ASSERT(config_ == nullptr, "Scheduler::AttachConfig",
               "scheduler config is already attached");

  ValidateSchedulerConfig(config);
  config_ = &config;
}

void Scheduler::DetachConfig() {
  COMET_ASSERT(!is_initialized_, "Scheduler::DetachConfig",
               "scheduler is still initialized");
  COMET_ASSERT(config_ != nullptr, "Scheduler::DetachConfig",
               "scheduler config is not attached");

  config_ = nullptr;
}

void Scheduler::Initialize() {
  COMET_ASSERT(!is_initialized_, "Scheduler::Initialize",
               "scheduler is already initialized");
  COMET_ASSERT(config_ != nullptr, "Scheduler::Initialize",
               "scheduler config is not attached");

  low_priority_queue_ = LockFreeMPMCRingQueue<JobDescr>{
      config_->job_queue_allocator, config_->job_queue_capacity};
  normal_priority_queue_ = LockFreeMPMCRingQueue<JobDescr>{
      config_->job_queue_allocator, config_->job_queue_capacity};
  high_priority_queue_ = LockFreeMPMCRingQueue<JobDescr>{
      config_->job_queue_allocator, config_->job_queue_capacity};
  io_queue_ = LockFreeMPMCRingQueue<IOJobDescr>{config_->job_queue_allocator,
                                                config_->job_queue_capacity};

  main_thread_queue_ = LockFreeMPMCRingQueue<MainThreadJobDescr>{
      config_->main_thread_queue_allocator,
      config_->main_thread_queue_capacity};

  fiber::AttachFiberStackAllocator(config_->fiber_stack_allocator);

  large_stack_fibers_.Initialize({
      .fiber_count = config_->large_fiber_count,
      .fiber_stack_size = fiber::kLargeStackSize,
      .queue_allocator = config_->fiber_queue_allocator,
      .fiber_allocator = config_->fiber_object_allocator,
  });

  gigantic_stack_fibers_.Initialize({
      .fiber_count = config_->gigantic_fiber_count,
      .fiber_stack_size = fiber::kGiganticStackSize,
      .queue_allocator = config_->fiber_queue_allocator,
      .fiber_allocator = config_->fiber_object_allocator,
  });

#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  external_library_stack_fibers_.Initialize({
      .fiber_count = config_->external_library_fiber_count,
      .fiber_stack_size = fiber::kNormalExternalLibraryStackSize,
      .queue_allocator = config_->fiber_queue_allocator,
      .fiber_allocator = config_->fiber_object_allocator,
  });
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

  counters_.Initialize({
      .counter_count = config_->counter_count,
      .queue_allocator = config_->counter_queue_allocator,
      .counter_allocator = config_->counter_allocator,
  });

  auto& fiber_life_cycle{fiber::FiberLifeCycleHandler::Get()};
  fiber_life_cycle.AttachAllocator(config_->fiber_life_cycle_allocator,
                                   config_->fiber_life_cycle_queue_capacity);
  fiber_life_cycle.Initialize();

  is_shutdown_required_.store(false, std::memory_order_release);

  const auto concurrent_thread_count{thread::GetConcurrentThreadCountLeft()};
  fiber_worker_count_ = config_->forced_fiber_worker_count;
  io_worker_count_ = config_->forced_io_worker_count;

  if (io_worker_count_ == 0) {
    io_worker_count_ = config_->default_io_worker_count;
  }

  if (fiber_worker_count_ == 0) {
    const auto thread_count{concurrent_thread_count};

    if (thread_count > io_worker_count_) {
      fiber_worker_count_ = thread_count - io_worker_count_;
    } else {
      COMET_LOG_WARNING(LoggerType::Core, "Scheduler::Initialize",
                        "io worker count is too high for current architecture",
                        "io_worker_count", io_worker_count_);
      fiber_worker_count_ = thread_count;
    }
  }

  COMET_LOG_INFO(LoggerType::Core, "Scheduler::Initialize",
                 "worker counts resolved", "fiber_worker_count",
                 fiber_worker_count_, "io_worker_count", io_worker_count_);

  fiber_workers_ = Array<FiberWorker>{config_->worker_allocator};
  fiber_workers_.Reserve(fiber_worker_count_);

  for (usize i{0}; i < fiber_worker_count_; ++i) {
    fiber_workers_.EmplaceLast(&config_->worker_lifecycle);
  }

  io_workers_ = Array<IOWorker>{config_->worker_allocator};
  io_workers_.Reserve(io_worker_count_);

  for (usize i{0}; i < io_worker_count_; ++i) {
    io_workers_.EmplaceLast(&config_->worker_lifecycle);
  }

  is_initialized_ = true;
}

void Scheduler::Shutdown() {
  COMET_ASSERT(is_initialized_, "Scheduler::Shutdown",
               "scheduler is not initialized");

  RequestShutdown();

  for (auto& fiber_worker : fiber_workers_) {
    fiber_worker.Stop();
  }

  for (auto& io_worker : io_workers_) {
    io_worker.Stop();
  }

  low_priority_queue_.Destroy();
  normal_priority_queue_.Destroy();
  high_priority_queue_.Destroy();
  io_queue_.Destroy();
  main_thread_queue_.Destroy();

  large_stack_fibers_.Destroy();
  gigantic_stack_fibers_.Destroy();
#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  external_library_stack_fibers_.Destroy();
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

  auto& fiber_life_cycle{fiber::FiberLifeCycleHandler::Get()};
  fiber_life_cycle.Shutdown();
  fiber_life_cycle.DetachAllocator();

  counters_.Destroy();

  fiber::DetachFiberStackAllocator();

  fiber_workers_.Release();
  io_workers_.Release();

  fiber_worker_count_ = 0;
  io_worker_count_ = 0;
  is_initialized_ = false;
}

void Scheduler::Run(const JobDescr& callback_descr,
                    bool is_main_thread_worker) {
  COMET_ASSERT(is_initialized_, "Scheduler::Run",
               "scheduler is not initialized");

  for (usize i{1}; i < fiber_worker_count_; ++i) {
    auto& fiber_worker{fiber_workers_[i]};
    fiber_worker.Run(&Scheduler::Work, this, &fiber_worker,
                     &Scheduler::WorkOnFibers);
  }

  for (usize i{0}; i < io_worker_count_; ++i) {
    auto& io_worker{io_workers_[i]};
    io_worker.Run(&Scheduler::Work, this, &io_worker, &Scheduler::WorkOnIO);
  }

  const auto worker_count{fiber_worker_count_ + io_worker_count_};

  while (GetCurrentWorkerCount() < worker_count - 1) {
    thread::Yield();
  }

  if (config_->is_main_thread_worker_disabled) {
    const auto& lifecycle{config_->worker_lifecycle};

    if (lifecycle.attach != nullptr) {
      lifecycle.attach();
    }

    Kick(callback_descr);

    if (is_main_thread_worker) {
      WorkFromMainThread();
    }

    if (lifecycle.detach != nullptr) {
      lifecycle.detach();
    }

    return;
  }

  Kick(callback_descr);

  if (is_main_thread_worker) {
    Work(&fiber_workers_[0], &Scheduler::WorkOnFibers);
  }
}

void Scheduler::RequestShutdown() {
  is_shutdown_required_.store(true, std::memory_order_release);

  for (usize i{0}; i < io_worker_count_; ++i) {
    io_worker_wakeup_.release();
  }
}

Counter* Scheduler::GenerateCounter() {
  COMET_ASSERT(is_initialized_, "Scheduler::GenerateCounter",
               "scheduler is not initialized");

  auto* counter{counters_.TryGet()};
  COMET_ASSERT(counter != nullptr, "Scheduler::GenerateCounter",
               "no counter is available");
  counter->Reset();
  return counter;
}

void Scheduler::DestroyCounter(Counter* counter) {
  COMET_ASSERT(is_initialized_, "Scheduler::DestroyCounter",
               "scheduler is not initialized");
  COMET_ASSERT(counter != nullptr, "Scheduler::DestroyCounter",
               "counter is null");

  counter->Reset();
  counters_.Push(counter);
}

void Scheduler::Wait(Counter* counter) {
  if (counter == nullptr) {
    return;
  }

  CounterWaiter waiter{*counter};
}

usize Scheduler::GetFiberWorkerCount() const noexcept {
  return fiber_worker_count_;
}

usize Scheduler::GetIOWorkerCount() const noexcept { return io_worker_count_; }

CounterGuard::CounterGuard() : counter_(Scheduler::Get().GenerateCounter()) {}

CounterGuard::~CounterGuard() { Scheduler::Get().DestroyCounter(counter_); }

void CounterGuard::Wait() { Scheduler::Get().Wait(counter_); }

Counter* CounterGuard::GetCounter() { return counter_; }
}  // namespace job
}  // namespace comet