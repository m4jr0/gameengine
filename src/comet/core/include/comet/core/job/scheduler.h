// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_JOB_SCHEDULER_H_
#define COMET_CORE_JOB_SCHEDULER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
#include <semaphore>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/container/array.h"
#include "comet/core/container/ring_queue.h"
#include "comet/core/essentials.h"
#include "comet/core/fiber/fiber.h"
#include "comet/core/job/job.h"
#include "comet/core/job/scheduler_config.h"
#include "comet/core/job/scheduler_pools.h"
#include "comet/core/job/worker.h"

namespace comet {
namespace job {
class Scheduler {
 public:
  static Scheduler& Get();

  Scheduler() = default;
  Scheduler(const Scheduler&) = delete;
  Scheduler(Scheduler&&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;
  Scheduler& operator=(Scheduler&&) = delete;
  ~Scheduler();

  void AttachConfig(const SchedulerConfig& config);
  void DetachConfig();

  void Initialize();
  void Shutdown();
  void Run(const JobDescr& callback_descr, bool is_main_thread_worker = true);
  void RequestShutdown();

  Counter* GenerateCounter();
  void DestroyCounter(Counter* counter);

  void Kick(const JobDescr& job_descr);
  void Kick(usize job_count, const JobDescr* job_descrs);
  void Kick(const IOJobDescr& job_descr);
  void Kick(usize job_count, const IOJobDescr* job_descrs);

  void Wait(Counter* counter);

  void KickAndWait(const JobDescr& job_descr);
  void KickAndWait(usize job_count, const JobDescr* job_descrs);
  void KickAndWait(const IOJobDescr& job_descr);
  void KickAndWait(usize job_count, const IOJobDescr* job_descrs);

  void KickOnMainThread(const MainThreadJobDescr& descr);

  usize GetFiberWorkerCount() const noexcept;
  usize GetIOWorkerCount() const noexcept;

 private:
  std::counting_semaphore<> io_worker_wakeup_{0};

  const SchedulerConfig* config_{nullptr};

  bool is_initialized_{false};
  usize fiber_worker_count_{0};
  usize io_worker_count_{0};

  internal::FiberPool large_stack_fibers_{};
  internal::FiberPool gigantic_stack_fibers_{};
#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  internal::FiberPool external_library_stack_fibers_{};
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

  internal::CounterPool counters_{};

  Array<FiberWorker> fiber_workers_{};
  Array<IOWorker> io_workers_{};

  static_assert(std::atomic<bool>::is_always_lock_free,
                "std::atomic<bool> must be always lock-free");
  std::atomic<bool> is_shutdown_required_{false};

  LockFreeMPMCRingQueue<JobDescr> low_priority_queue_{};
  LockFreeMPMCRingQueue<JobDescr> normal_priority_queue_{};
  LockFreeMPMCRingQueue<JobDescr> high_priority_queue_{};
  LockFreeMPMCRingQueue<IOJobDescr> io_queue_{};

  LockFreeMPMCRingQueue<MainThreadJobDescr> main_thread_queue_{};

  using WorkFunc = void (Scheduler::*)();

  void Work(Worker* worker, WorkFunc work_func);
  void WorkOnFibers();
  void WorkOnIO();

  internal::FiberPool* ResolveFiberPool(const JobDescr& job_descr);
  void CleanCompletedAndTryResumeNext();

  static void OnFiberEnd(fiber::Fiber* fiber, void* data);

  void SubmitJob(const JobDescr& job_descr);
  void SubmitJob(const IOJobDescr& job_descr);

  void PromoteJobs();

  bool TryAcquireRunnableJobFromQueue(LockFreeMPMCRingQueue<JobDescr>& queue,
                                      JobDescr& job_descr,
                                      fiber::Fiber*& fiber);

  bool TryAcquireRunnableJob(JobDescr& job_descr, fiber::Fiber*& fiber);
  void RequeueJob(const JobDescr& job_descr);
  bool TryRequeueJob(const JobDescr& job_descr);

  void WorkFromMainThread();
};

class CounterGuard {
 public:
  CounterGuard();
  ~CounterGuard();
  CounterGuard(const CounterGuard&) = delete;
  CounterGuard& operator=(const CounterGuard&) = delete;

  void Wait();
  Counter* GetCounter();

 private:
  Counter* counter_{nullptr};
};
}  // namespace job
}  // namespace comet

#endif  // COMET_CORE_JOB_SCHEDULER_H_