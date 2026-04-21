// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "scheduler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <optional>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/concurrency/fiber/fiber_context.h"
#include "comet/core/concurrency/fiber/fiber_life_cycle.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/job_label.h"
#include "comet/core/concurrency/job/worker_context.h"
#include "comet/core/concurrency/thread/thread_context.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/core/logger/logging.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type_trait.h"
#include "comet/time/chrono.h"

namespace comet {
namespace job {
namespace internal {
FiberPool::FiberPool(usize fiber_count, usize fiber_stack_size)
    : initial_fiber_count_{fiber_count}, fiber_stack_size_{fiber_stack_size} {}

void FiberPool::Initialize() {
  fibers_ = LockFreeMPMCRingQueue<fiber::Fiber*>{&queue_allocator_,
                                                 initial_fiber_count_};

  fiber_allocator_ = memory::PlatformStackAllocator{
      initial_fiber_count_ * sizeof(fiber::Fiber) + alignof(fiber::Fiber),
      memory::kEngineMemoryTagFiber};
  fiber_allocator_.Initialize();

  for (usize i{0}; i < initial_fiber_count_; ++i) {
    auto* fiber{fiber_allocator_.AllocateOneAndPopulate<fiber::Fiber>(
        fiber_stack_size_)};
    fiber->Initialize();
    fibers_.Push(fiber);
  }
}

void FiberPool::Destroy() {
  for (;;) {
    const auto fiber_box{fibers_.TryPop()};

    if (!fiber_box.has_value()) {
      break;
    }

    fiber_box.value()->Destroy();
  }

  fibers_.Destroy();
  fiber_allocator_.Destroy();
}

fiber::Fiber* FiberPool::TryPop() {
  const auto fiber_box{fibers_.TryPop()};
  fiber::Fiber* fiber{fiber_box.value_or(nullptr)};

  if (fiber != nullptr) {
    fiber->Reset();
  }

  return fiber;
}

void FiberPool::Push(fiber::Fiber* fiber) {
  COMET_ASSERT(fiber->GetStackCapacity() == fiber_stack_size_,
               "job::internal::FiberPool::Push",
               "fiber stack capacity mismatch", "stack_capacity",
               fiber->GetStackCapacity(), "expected_stack_capacity",
               fiber_stack_size_);
  fibers_.Push(fiber);
}

usize FiberPool::GetTotalAllocatedStackSize() const {
  return (initial_fiber_count_ + alignof(fiber::Fiber::Stack)) *
         fiber::Fiber::GetAllocatedStackSize(fiber_stack_size_);
}

void CounterPool::Initialize() {
  counters_ = LockFreeMPMCRingQueue<Counter*>{
      &queue_allocator_,
      static_cast<usize>(COMET_CONF_U16(conf::kCoreJobCounterCount))};

  const auto capacity{counters_.GetCapacity()};
  counter_allocator_ = memory::PlatformStackAllocator{
      sizeof(Counter) * capacity + alignof(Counter),
      memory::kEngineMemoryTagFiber};
  counter_allocator_.Initialize();

  for (usize i{0}; i < capacity; ++i) {
    auto* counter{counter_allocator_.AllocateOneAndPopulate<Counter>()};
    new (counter) Counter{};
    counters_.Push(counter);
  }
}

void CounterPool::Destroy() {
  counters_.Destroy();
  counter_allocator_.Destroy();
}

Counter* CounterPool::TryGet() {
  const auto counter_box{counters_.TryPop()};
  return counter_box.value_or(nullptr);
}

void CounterPool::Push(Counter* counter) { counters_.Push(counter); }
}  // namespace internal

Scheduler& Scheduler::Get() {
  static Scheduler singleton{};
  return singleton;
}

Scheduler::~Scheduler() {
  COMET_ASSERT(!is_initialized_, "Scheduler::~Scheduler",
               "scheduler is still initialized");
}

void Scheduler::Initialize() {
  COMET_ASSERT(!is_initialized_, "Scheduler::Initialize",
               "scheduler is already initialized");

  low_priority_queue_ = LockFreeMPMCRingQueue<JobDescr>{
      &job_queue_allocator_,
      static_cast<usize>(COMET_CONF_U16(conf::kCoreJobQueueCount))};
  normal_priority_queue_ = LockFreeMPMCRingQueue<JobDescr>{
      &job_queue_allocator_,
      static_cast<usize>(COMET_CONF_U16(conf::kCoreJobQueueCount))};
  high_priority_queue_ = LockFreeMPMCRingQueue<JobDescr>{
      &job_queue_allocator_,
      static_cast<usize>(COMET_CONF_U16(conf::kCoreJobQueueCount))};
  io_queue_ = LockFreeMPMCRingQueue<IOJobDescr>{
      &job_queue_allocator_,
      static_cast<usize>(COMET_CONF_U16(conf::kCoreJobQueueCount))};

  fiber::AllocateFiberStackMemory(
      large_stack_fibers_.GetTotalAllocatedStackSize() +
      large_stack_fibers_.GetTotalAllocatedStackSize()
#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
      + external_library_stack_fibers_.GetTotalAllocatedStackSize()
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  );

  large_stack_fibers_.Initialize();
  gigantic_stack_fibers_.Initialize();
#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  external_library_stack_fibers_.Initialize();
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

  counters_.Initialize();
  is_shutdown_required_.store(false, std::memory_order_release);

  const auto concurrent_thread_count{thread::GetConcurrentThreadCountLeft()};
  fiber_worker_count_ = COMET_CONF_U8(conf::kCoreForcedFiberWorkerCount);
  io_worker_count_ = COMET_CONF_U8(conf::kCoreForcedIOWorkerCount);

  if (io_worker_count_ == 0) {
    io_worker_count_ = kDefaultIOWorkerCount;
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
  fiber_workers_ = Array<FiberWorker>{&worker_allocator};
  fiber_workers_.Resize(fiber_worker_count_);
  io_workers_ = Array<IOWorker>{&worker_allocator};
  io_workers_.Resize(io_worker_count_);

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

  large_stack_fibers_.Destroy();
  gigantic_stack_fibers_.Destroy();
#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  external_library_stack_fibers_.Destroy();
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

  counters_.Destroy();
  fiber::DestroyFiberStackMemory();

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

  // Subtract one because of main thread's worker.
  while (GetCurrentWorkerCount() < worker_count - 1) {
    thread::Yield();
  }

#ifdef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  if (IsMainThreadWorkerDisabled()) {
    // Case: main thread. We only need to attach its worker since the thread is
    // already running.
    fiber_workers_[0].Attach();
    Scheduler::Get().Kick(callback_descr);

    if (is_main_thread_worker) {
      WorkFromMainThread();
    }
    return;
  }
#else
  COMET_ASSERT(!IsMainThreadWorkerDisabled(), "Scheduler::Run",
               "main thread worker cannot be disabled");
#endif  // COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER

  // From this point onward, the rest of the code must be fully jobified.
  // Otherwise, a stack overflow or memory corruption could occur, as
  // non-jobified code may require a large stack that exceeds the capacity of
  // our fibers.
  Scheduler::Get().Kick(callback_descr);

  if (is_main_thread_worker) {
    Work(&fiber_workers_[0],
         &Scheduler::WorkOnFibers);  // Main thread now behaves as a worker.
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

void Scheduler::Kick(const JobDescr& job_descr) {
  COMET_ASSERT(is_initialized_, "Scheduler::Kick",
               "scheduler is not initialized");
  SubmitJob(job_descr);
}

void Scheduler::Kick(usize job_count, const JobDescr* job_descrs) {
  COMET_ASSERT(is_initialized_, "Scheduler::Kick",
               "scheduler is not initialized");

  for (usize i{0}; i < job_count; ++i) {
    Kick(job_descrs[i]);
  }
}

void Scheduler::Kick(const IOJobDescr& job_descr) {
  COMET_ASSERT(is_initialized_, "Scheduler::Kick",
               "scheduler is not initialized");
  SubmitJob(job_descr);
}

void Scheduler::Kick(usize job_count, const IOJobDescr* job_descrs) {
  COMET_ASSERT(is_initialized_, "Scheduler::Kick",
               "scheduler is not initialized");

  for (usize i{0}; i < job_count; ++i) {
    Kick(job_descrs[i]);
  }
}

void Scheduler::Wait(Counter* counter) {
  if (counter == nullptr) {
    return;
  }

  CounterWaiter waiter{*counter};
}

void Scheduler::KickAndWait(const JobDescr& job_descr) {
  COMET_ASSERT(is_initialized_, "Scheduler::KickAndWait",
               "scheduler is not initialized");
  Kick(job_descr);
  Wait(job_descr.counter);
}

void Scheduler::KickAndWait(usize job_count, const JobDescr* job_descrs) {
  COMET_ASSERT(is_initialized_, "Scheduler::KickAndWait",
               "scheduler is not initialized");
  Kick(job_count, job_descrs);

  for (usize i{0}; i < job_count; ++i) {
    Wait(job_descrs[i].counter);
  }
}

void Scheduler::KickAndWait(const IOJobDescr& job_descr) {
  COMET_ASSERT(is_initialized_, "Scheduler::KickAndWait",
               "scheduler is not initialized");
  Kick(job_descr);
  Wait(job_descr.counter);
}

void Scheduler::KickAndWait(usize job_count, const IOJobDescr* job_descrs) {
  COMET_ASSERT(is_initialized_, "Scheduler::KickAndWait",
               "scheduler is not initialized");
  Kick(job_count, job_descrs);

  for (usize i{0}; i < job_count; ++i) {
    Wait(job_descrs[i].counter);
  }
}

#ifdef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
static LockFreeMPMCRingQueue<MainThreadJobDescr> main_thread_queue{};
#endif  // COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER

void Scheduler::KickOnMainThread(
    [[maybe_unused]] const MainThreadJobDescr& descr) {
#ifdef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  descr.counter->Increment();
  main_thread_queue.Push(descr);
#endif  // COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
}

usize Scheduler::GetFiberWorkerCount() const noexcept {
  return fiber_worker_count_;
}

usize Scheduler::GetIOWorkerCount() const noexcept { return io_worker_count_; }

void Scheduler::Work(Worker* worker, WorkFunc work_func) {
  COMET_ASSERT(worker != nullptr, "Scheduler::Work", "worker is null");

  COMET_ASSERT(work_func != nullptr, "Scheduler::Work",
               "work function is null");

  worker->Attach();
  (this->*work_func)();
  worker->Detach();
}

void Scheduler::WorkOnFibers() {
  time::Chrono chrono{};
  chrono.Start(promotion_interval_);

  constexpr usize kIdleYieldCount{64};
  constexpr auto kIdleSleepDuration{std::chrono::microseconds{100}};
  usize idle_count{0};

  while (!is_shutdown_required_.load(std::memory_order_relaxed)) {
    if (chrono.IsFinished()) {
      PromoteJobs();
      chrono.Restart();
    }

    CleanCompletedAndTryResumeNext();

    JobDescr job_descr{};
    fiber::Fiber* fiber{nullptr};

    if (!TryAcquireRunnableJob(job_descr, fiber)) {
      CleanCompletedAndTryResumeNext();

      if (idle_count < kIdleYieldCount) {
        ++idle_count;
        thread::Yield();
      } else {
        std::this_thread::sleep_for(kIdleSleepDuration);
        idle_count = 0;
      }

      continue;
    }

    idle_count = 0;

    fiber->Attach(job_descr.entry_point, job_descr.params_handle, OnFiberEnd,
                  job_descr.counter
#ifdef COMET_FIBER_DEBUG_LABEL
                  ,
                  job_descr.debug_label
#endif  // COMET_FIBER_DEBUG_LABEL
    );

    fiber::internal::RunOrResume(fiber);
    CleanCompletedAndTryResumeNext();
  }
}

void Scheduler::WorkOnIO() {
  while (true) {
    io_worker_wakeup_.acquire();

    if (is_shutdown_required_.load(std::memory_order_acquire)) {
      break;
    }

    for (;;) {
      const auto job_box{io_queue_.TryPop()};

      if (!job_box.has_value()) {
        break;
      }

      auto& job{job_box.value()};
      job.entry_point(job.params_handle);

      if (job.counter != nullptr) {
        job.counter->Decrement();
      }
    }
  }
}

internal::FiberPool* Scheduler::ResolveFiberPool(const JobDescr& job_descr) {
  internal::FiberPool* fibers{nullptr};

  switch (job_descr.stack_size) {
    case JobStackSize::Normal:
      fibers = &large_stack_fibers_;
      break;

    case JobStackSize::Large:
      fibers = &gigantic_stack_fibers_;
      break;

#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
    case JobStackSize::ExternalLibrary:
      fibers = &external_library_stack_fibers_;
      break;
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

    default:
      COMET_ASSERT(false, "Scheduler::ResolveFiberPool",
                   "job stack size is invalid", "stack_size",
                   GetJobStackSizeLabel(job_descr.stack_size),
                   "stack_size_value", ToUnderlying(job_descr.stack_size));
      break;
  }

  return fibers;
}

void Scheduler::CleanCompletedAndTryResumeNext() {
  fiber::Fiber* completed_fiber;
  auto& life_cycle_handler{fiber::FiberLifeCycleHandler::Get()};

  while ((completed_fiber = life_cycle_handler.TryGetCompleted()) != nullptr) {
    internal::FiberPool* fibers{nullptr};

    switch (completed_fiber->GetStackCapacity()) {
      case fiber::kLargeStackSize:
        fibers = &large_stack_fibers_;
        break;

      case fiber::kGiganticStackSize:
        fibers = &gigantic_stack_fibers_;
        break;

#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
      case fiber::kNormalExternalLibraryStackSize:
        fibers = &external_library_stack_fibers_;
        break;
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

      default:
        COMET_ASSERT(false, "Scheduler::CleanCompletedAndTryResumeNext",
                     "fiber stack capacity is invalid", "stack_capacity",
                     completed_fiber->GetStackCapacity());
        break;
    }

    fibers->Push(completed_fiber);
  }

  auto* sleeping_fiber{life_cycle_handler.TryWakingUp()};

  if (sleeping_fiber != nullptr) {
    fiber::internal::RunOrResume(sleeping_fiber);
  }
}

void Scheduler::OnFiberEnd(fiber::Fiber* fiber, void* data) {
  auto* counter{static_cast<Counter*>(data)};
  fiber->Detach();
  auto& life_cycle_handler{fiber::FiberLifeCycleHandler::Get()};
  life_cycle_handler.PutToCompleted(fiber);

  if (counter != nullptr) {
    counter->Decrement();
  }

  fiber::internal::ResumeWorker();
}

void Scheduler::SubmitJob(const JobDescr& job_descr) {
  if (job_descr.counter != nullptr) {
    job_descr.counter->Increment();
  }

  switch (job_descr.priority) {
    case JobPriority::High:
      high_priority_queue_.Push(job_descr);
      break;
    case JobPriority::Normal:
      normal_priority_queue_.Push(job_descr);
      break;
    case JobPriority::Low:
      low_priority_queue_.Push(job_descr);
      break;
    default:
      COMET_ASSERT(false, "Scheduler::SubmitJob", "job priority is invalid",
                   "priority", GetJobPriorityLabel(job_descr.priority),
                   "priority_value", ToUnderlying(job_descr.priority));
      break;
  }
}

void Scheduler::SubmitJob(const IOJobDescr& job_descr) {
  if (job_descr.counter != nullptr) {
    job_descr.counter->Increment();
  }

  io_queue_.Push(job_descr);
  io_worker_wakeup_.release();
}

void Scheduler::PromoteJobs() {
  std::optional<JobDescr> job_box{normal_priority_queue_.TryPop()};

  while (job_box.has_value()) {
    high_priority_queue_.Push(job_box.value());
    job_box = normal_priority_queue_.TryPop();
  }

  job_box = low_priority_queue_.TryPop();

  while (job_box.has_value()) {
    normal_priority_queue_.Push(job_box.value());
    job_box = low_priority_queue_.TryPop();
  }
}

bool Scheduler::TryAcquireRunnableJobFromQueue(
    LockFreeMPMCRingQueue<JobDescr>& queue, JobDescr& job_descr,
    fiber::Fiber*& fiber) {
  const auto job_box{queue.TryPop()};

  if (!job_box.has_value()) {
    return false;
  }

  job_descr = job_box.value();
  auto* fibers{ResolveFiberPool(job_descr)};

  COMET_ASSERT(fibers != nullptr, "Scheduler::TryAcquireRunnableJobFromQueue",
               "fiber pool could not be resolved");

  fiber = fibers->TryPop();

  if (fiber != nullptr) {
    return true;
  }

  RequeueJob(job_descr);
  return false;
}

bool Scheduler::TryAcquireRunnableJob(JobDescr& job_descr,
                                      fiber::Fiber*& fiber) {
  fiber = nullptr;

  if (TryAcquireRunnableJobFromQueue(high_priority_queue_, job_descr, fiber)) {
    return true;
  }

  if (TryAcquireRunnableJobFromQueue(normal_priority_queue_, job_descr,
                                     fiber)) {
    return true;
  }

  return TryAcquireRunnableJobFromQueue(low_priority_queue_, job_descr, fiber);
}

void Scheduler::RequeueJob(const JobDescr& job_descr) {
  switch (job_descr.priority) {
    case JobPriority::High:
      high_priority_queue_.Push(job_descr);
      break;

    case JobPriority::Normal:
      normal_priority_queue_.Push(job_descr);
      break;

    case JobPriority::Low:
      low_priority_queue_.Push(job_descr);
      break;

    default:
      COMET_ASSERT(false, "Scheduler::RequeueJob", "job priority is invalid",
                   "priority", GetJobPriorityLabel(job_descr.priority),
                   "priority_value", ToUnderlying(job_descr.priority));
      break;
  }
}

#ifdef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
void Scheduler::WorkFromMainThread() {
  memory::PlatformAllocator allocator{memory::kEngineMemoryTagMainThread};
  constexpr auto kMaxMainThreadJobCount{
      16};  // Consider exposing this as a configurable setting.

  main_thread_queue = LockFreeMPMCRingQueue<MainThreadJobDescr>{
      &allocator, kMaxMainThreadJobCount};

  while (!is_shutdown_required_.load(std::memory_order_relaxed)) {
    const auto job_box{main_thread_queue.TryPop()};

    if (!job_box.has_value()) {
      continue;
    }

    auto& job{job_box.value()};
    job.entry_point(job.params_handle);

    if (job.counter != nullptr) {
      job.counter->Decrement();
    }
  }

  main_thread_queue.Destroy();
}
#endif  // COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER

CounterGuard::CounterGuard() : counter_(Scheduler::Get().GenerateCounter()) {}

CounterGuard::~CounterGuard() { Scheduler::Get().DestroyCounter(counter_); }

void CounterGuard::Wait() { Scheduler::Get().Wait(counter_); }

Counter* CounterGuard::GetCounter() { return counter_; }
}  // namespace job
}  // namespace comet