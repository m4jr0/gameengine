// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/job/scheduler.h"
////////////////////////////////////////////////////////////////////////////////

#include <utility>

#include "comet/core/fiber/fiber_utils.h"
#include "comet/core/job/job_label.h"
#include "comet/core/type_trait.h"

namespace comet {
namespace job {
void Scheduler::Kick(const JobDescr& job_descr) {
  COMET_ASSERT(is_initialized_, "Scheduler::Kick",
               "scheduler is not initialized");
  SubmitJob(job_descr);
}

void Scheduler::Kick(usize job_count, const JobDescr* job_descrs) {
  COMET_ASSERT(is_initialized_, "Scheduler::Kick",
               "scheduler is not initialized");
  COMET_ASSERT(job_descrs != nullptr, "Scheduler::Kick",
               "job descriptors are null");

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
  COMET_ASSERT(job_descrs != nullptr, "Scheduler::Kick",
               "io job descriptors are null");

  for (usize i{0}; i < job_count; ++i) {
    Kick(job_descrs[i]);
  }
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
  COMET_ASSERT(job_descrs != nullptr, "Scheduler::KickAndWait",
               "job descriptors are null");

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
  COMET_ASSERT(job_descrs != nullptr, "Scheduler::KickAndWait",
               "io job descriptors are null");

  Kick(job_count, job_descrs);

  for (usize i{0}; i < job_count; ++i) {
    Wait(job_descrs[i].counter);
  }
}

void Scheduler::KickOnMainThread(
    [[maybe_unused]] const MainThreadJobDescr& descr) {
#ifdef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  COMET_ASSERT(is_initialized_, "Scheduler::KickOnMainThread",
               "scheduler is not initialized");

  descr.counter->Increment();

  while (!main_thread_queue_.TryPush(descr)) {
    fiber::Yield();
  }
#endif  // COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
}

void Scheduler::SubmitJob(const JobDescr& job_descr) {
  if (job_descr.counter != nullptr) {
    job_descr.counter->Increment();
  }

  switch (job_descr.priority) {
    case JobPriority::High:
      while (!high_priority_queue_.TryPush(job_descr)) {
        fiber::Yield();
      }
      break;

    case JobPriority::Normal:
      while (!normal_priority_queue_.TryPush(job_descr)) {
        fiber::Yield();
      }
      break;

    case JobPriority::Low:
      while (!low_priority_queue_.TryPush(job_descr)) {
        fiber::Yield();
      }
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

  while (!io_queue_.TryPush(job_descr)) {
    fiber::Yield();
  }

  io_worker_wakeup_.release();
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
  while (!TryRequeueJob(job_descr)) {
    CleanCompletedAndTryResumeNext();
    fiber::Yield();
  }
}

bool Scheduler::TryRequeueJob(const JobDescr& job_descr) {
  switch (job_descr.priority) {
    case JobPriority::High:
      return high_priority_queue_.TryPush(job_descr);

    case JobPriority::Normal:
      return normal_priority_queue_.TryPush(job_descr);

    case JobPriority::Low:
      return low_priority_queue_.TryPush(job_descr);

    default:
      COMET_ASSERT(false, "Scheduler::TryRequeueJob", "job priority is invalid",
                   "priority", GetJobPriorityLabel(job_descr.priority),
                   "priority_value", ToUnderlying(job_descr.priority));
      return false;
  }
}
}  // namespace job
}  // namespace comet