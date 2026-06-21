// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/job/scheduler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <chrono>
#include <thread>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/fiber/fiber.h"
#include "comet/core/fiber/fiber_context.h"
#include "comet/core/fiber/fiber_life_cycle.h"
#include "comet/core/fiber/fiber_utils.h"
#include "comet/core/job/job_label.h"
#include "comet/core/thread/thread_context.h"
#include "comet/core/time/chrono.h"
#include "comet/core/type_trait.h"

namespace comet {
namespace job {
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
  chrono.Start(config_->promotion_interval);

#ifdef COMET_ALLOW_WORKER_SLEEP
  constexpr usize kIdleYieldCount{64};
  constexpr auto kIdleSleepDuration{std::chrono::nanoseconds{500}};
  usize idle_count{0};
#endif  // COMET_ALLOW_WORKER_SLEEP

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

#ifdef COMET_ALLOW_WORKER_SLEEP
      if (idle_count < kIdleYieldCount) {
        ++idle_count;
      } else {
        std::this_thread::sleep_for(kIdleSleepDuration);
        idle_count = 0;
      }
#endif  // COMET_ALLOW_WORKER_SLEEP

      continue;
    }

#ifdef COMET_ALLOW_WORKER_SLEEP
    idle_count = 0;
#endif  // COMET_ALLOW_WORKER_SLEEP

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
  fiber::Fiber* completed_fiber{nullptr};
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
  fiber::FiberLifeCycleHandler::Get().PutToCompleted(fiber);

  if (counter != nullptr) {
    counter->Decrement();
  }

  fiber::internal::ResumeWorker();
}

void Scheduler::PromoteJobs() {
  auto job_box{normal_priority_queue_.TryPop()};

  while (job_box.has_value()) {
    while (!high_priority_queue_.TryPush(std::move(job_box.value()))) {
      fiber::Yield();
    }

    job_box = normal_priority_queue_.TryPop();
  }

  job_box = low_priority_queue_.TryPop();

  while (job_box.has_value()) {
    while (!normal_priority_queue_.TryPush(std::move(job_box.value()))) {
      fiber::Yield();
    }

    job_box = low_priority_queue_.TryPop();
  }
}

void Scheduler::WorkFromMainThread() {
  while (!is_shutdown_required_.load(std::memory_order_relaxed)) {
    const auto job_box{main_thread_queue_.TryPop()};

    if (!job_box.has_value()) {
      thread::Yield();
      continue;
    }

    auto& job{job_box.value()};
    job.entry_point(job.params_handle);

    if (job.counter != nullptr) {
      job.counter->Decrement();
    }
  }
}
}  // namespace job
}  // namespace comet