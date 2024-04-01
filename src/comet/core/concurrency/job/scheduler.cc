// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "scheduler.h"

#include <fstream>
#include <optional>

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/concurrency/fiber/fiber_context.h"
#include "comet/core/concurrency/fiber/fiber_queue.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/thread/thread_utils.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/core/logger.h"
#include "comet/time/chrono.h"

namespace comet {
namespace job {
namespace internal {
void SchedulerStarterBarrier::Wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  cv_.wait(lock, [this] { return is_ready_; });
}

void SchedulerStarterBarrier::Unleash() {
  std::lock_guard<std::mutex> lock(mutex_);
  is_ready_ = true;
  cv_.notify_all();
}

bool SchedulerStarterBarrier::IsReady() const noexcept { return is_ready_; }
}  // namespace internal

Scheduler& Scheduler::Get() {
  static Scheduler singleton{};
  return singleton;
}

void Scheduler::Initialize() {
  auto large_stack_fiber_count{COMET_CONF_U16(conf::kCoreLargeFiberCount)};
  large_stack_fibers_.reserve(large_stack_fiber_count);

  for (usize i{0}; i < large_stack_fiber_count; ++i) {
    auto* fiber{
        memory::AllocateAligned<fiber::Fiber>(memory::MemoryTag::Fiber)};
    new (fiber) fiber::Fiber{fiber::kLargeStack};
    fiber->Initialize();
    large_stack_fibers_.emplace_back(fiber);
  }

  auto gigantic_stack_fiber_count{
      COMET_CONF_U16(conf::kCoreGiganticFiberCount)};
  gigantic_stack_fibers_.reserve(gigantic_stack_fiber_count);

  for (usize i{0}; i < gigantic_stack_fiber_count; ++i) {
    auto* fiber{
        memory::AllocateAligned<fiber::Fiber>(memory::MemoryTag::Fiber)};
    new (fiber) fiber::Fiber{fiber::kGiganticStack};
    gigantic_stack_fibers_.emplace_back(fiber);
  }

  auto counter_count{COMET_CONF_U16(conf::kCoreJobCounterCount)};

  for (usize i{0}; i < counter_count; ++i) {
    auto* counter{memory::AllocateAligned<Counter>(memory::MemoryTag::Fiber)};
    new (counter) Counter{};
    available_counters_.Push(counter);
  }

  is_shutdown_required_ = false;
  worker_count_ = COMET_CONF_U8(conf::kCoreForcedWorkerCount);
  io_worker_count_ = COMET_CONF_U8(conf::kCoreForcedIOWorkerCount);

  if (io_worker_count_ == 0) {
    io_worker_count_ = kDefaultIOWorkerCount;
  }

  if (worker_count_ == 0) {
    auto thread_count{thread::GetConcurrentThreadCount()};

    if (thread_count > io_worker_count_) {
      worker_count_ = thread_count - io_worker_count_;
    } else {
      COMET_LOG_CORE_WARNING(
          "I/O worker count is too high on this architecture (",
          io_worker_count_, "). Oversubscription will occur.");
      worker_count_ = thread_count;
    }
  }

  COMET_LOG_CORE_INFO("Worker count: ", worker_count_,
                      ", I/O worker count: ", io_worker_count_, ".");
  workers_.reserve(worker_count_);
  io_workers_.reserve(io_worker_count_);
}

void Scheduler::Shutdown() {
  is_shutdown_required_ = true;

  for (auto& worker : workers_) {
    worker.Destroy();
  }

  for (auto& worker : io_workers_) {
    worker.Destroy();
  }

  for (usize i{0}; i < large_stack_fibers_.size(); ++i) {
    auto* fiber{large_stack_fibers_[i]};

    if (fiber == nullptr) {
      continue;
    }

    fiber->Destroy();
  }

  large_stack_fibers_.clear();

  for (usize i{0}; i < gigantic_stack_fibers_.size(); ++i) {
    auto* fiber{gigantic_stack_fibers_[i]};

    if (fiber == nullptr) {
      continue;
    }

    fiber->Destroy();
  }

  gigantic_stack_fibers_.clear();

  for (usize i{0}; i < available_counters_.GetSize(); ++i) {
    auto* counter{available_counters_.Get()};
    available_counters_.Pop();
    memory::Deallocate(counter);
  }
}

void Scheduler::Start(const JobDescr& callback_descr) {
  COMET_ASSERT(!starter_barrier_.IsReady(),
               "Scheduler appears to have started already!");

  workers_.emplace_back(std::thread{});

  for (usize i{1}; i < worker_count_; ++i) {
    workers_.emplace_back(std::thread{&Scheduler::WorkerThread, this, i});
  }

  for (usize i{0}; i < io_worker_count_; ++i) {
    io_workers_.emplace_back(std::thread{&Scheduler::IOWorkerThread, this});
  }

  starter_barrier_.Unleash();
  workers_[0].Initialize();
  // TODO(m4jr0): Uncomment code and remove the manual callback invocation.
  // This should only be done after the engine has been fully jobified.
  // Uncommenting this prematurely increases the risk of memory corruption, as
  // a fiber's stack size may be insufficient to handle the remaining
  // execution of the engine.
  //
  // Scheduler::Get().Kick(callback_descr);
  // Work();  // Main thread now behaves as a worker.
  callback_descr.entry_point(callback_descr.params_handle);
}

Counter* Scheduler::AllocateCounter() {
  fiber::SimpleLockGuard lock{counter_lock_};
  COMET_ASSERT(available_counters_.GetSize() > 0,
               "No counter is available anymore!");
  auto* counter{available_counters_.Get()};
  available_counters_.Pop();
  counter->Reset();
  return counter;
}

void Scheduler::FreeCounter(Counter* counter) {
  COMET_ASSERT(counter != nullptr, "Counter provided is null!");
  fiber::SimpleLockGuard lock{counter_lock_};
  counter->Reset();
  available_counters_.Push(counter);
}

void Scheduler::Kick(const JobDescr& job_descr) { SubmitJob(job_descr); }

void Scheduler::Kick(usize job_count, const JobDescr* job_descrs) {
  for (usize i{0}; i < job_count; ++i) {
    Kick(job_descrs[i]);
  }
}

void Scheduler::Kick(const IOJobDescr& job_descr) { SubmitJob(job_descr); }

void Scheduler::Kick(usize job_count, const IOJobDescr* job_descrs) {
  for (usize i{0}; i < job_count; ++i) {
    Kick(job_descrs[i]);
  }
}

void Scheduler::Wait(Counter* counter) { CounterGuard guard{*counter}; }

void Scheduler::KickAndWait(const JobDescr& job_descr) {
  Kick(job_descr);
  Wait(job_descr.counter);
}

void Scheduler::KickAndWait(usize job_count, const JobDescr* job_descrs) {
  Kick(job_count, job_descrs);

  for (usize i{0}; i < job_count; ++i) {
    Wait(job_descrs[i].counter);
  }
}

void Scheduler::WorkerThread(usize worker_index) {
  starter_barrier_.Wait();
  auto& worker{workers_[worker_index]};
  worker.Initialize();
  Work();
}

void Scheduler::Work() {
  time::Chrono chrono{};
  chrono.Start(promotion_interval_);

  while (!is_shutdown_required_) {
    if (chrono.IsFinished()) {
      PromoteJobs();
      chrono.Restart();
    }

    std::optional<JobDescr> job_box{high_priority_queue_.Pop()};

    if (!job_box.has_value()) {
      job_box = normal_priority_queue_.Pop();
    }

    if (!job_box.has_value()) {
      job_box = low_priority_queue_.Pop();
    }

    if (!job_box.has_value()) {
      fiber::Yield();
      continue;
    }

    auto& job{job_box.value()};
    auto* fiber{ResolveFiber(job)};
    COMET_ASSERT(fiber != nullptr, "No fiber available!");
    fiber->Attach(job.entry_point, job.params_handle, OnFiberEnd, job.counter);
    auto& fiber_queue{fiber::FiberQueue::Get()};
    fiber_queue.Push(fiber);

    fiber::Yield();
  }
}

void Scheduler::IOWorkerThread() {
  while (!is_shutdown_required_) {
    if (is_shutdown_required_) {
      break;
    }

    auto job_box{io_queue_.Pop()};

    if (!job_box.has_value()) {
      continue;
    }

    auto& job{job_box.value()};
    job.entry_point(job.params_handle);
    job.counter->Decrement();
  }
}

fiber::Fiber* Scheduler::ResolveFiber(const JobDescr& job_descr) {
  fiber::FiberLockGuard lock{fiber_pool_mutex_};
  std::vector<fiber::Fiber*>* fibers{nullptr};

  switch (job_descr.stack_size) {
    case JobStackSize::Normal:
      fibers = &large_stack_fibers_;
      break;

    case JobStackSize::Large:
      fibers = &gigantic_stack_fibers_;
      break;

    default:
      COMET_ASSERT(false, "Unknown or unsupported job stack size: ",
                   GetJobStackSizeLabel(job_descr.stack_size), "!");
  }

  COMET_ASSERT(fibers != nullptr, "Could not resolve which fiber to use!");
  fiber::Fiber* fiber{fibers->back()};
  fibers->pop_back();
  fiber->Reset();
  return fiber;
}

void Scheduler::OnFiberEnd(fiber::Fiber* fiber, void* data) {
  auto* counter{static_cast<Counter*>(data)};
  auto& scheduler{Get()};

  {
    fiber::FiberLockGuard lock{scheduler.fiber_pool_mutex_};
    fiber->Detach();
    scheduler.large_stack_fibers_.emplace_back(fiber);
  }

  counter->Decrement();
  fiber::SwitchTo(fiber::FiberQueue::Get().Pop());
}

void Scheduler::SubmitJob(const JobDescr& job_descr) {
  COMET_ASSERT(job_descr.counter != nullptr,
               "Counter of job provided is null!");
  job_descr.counter->Increment();

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
      COMET_ASSERT(
          false, "Unknown or unsupported job priority: ",
          static_cast<std::underlying_type_t<JobPriority>>(job_descr.priority),
          "!");
      break;
  }
}

void Scheduler::SubmitJob(const IOJobDescr& job_descr) {
  job_descr.counter->Increment();
  io_queue_.Push(job_descr);
}

void Scheduler::PromoteJobs() {
  std::optional<JobDescr> job_box{normal_priority_queue_.Pop()};

  while (job_box.has_value()) {
    high_priority_queue_.Push(job_box.value());
    job_box = normal_priority_queue_.Pop();
  }

  job_box = low_priority_queue_.Pop();

  while (job_box.has_value()) {
    normal_priority_queue_.Push(job_box.value());
    job_box = low_priority_queue_.Pop();
  }
}
}  // namespace job
}  // namespace comet