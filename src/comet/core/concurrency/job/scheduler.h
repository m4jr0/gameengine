// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_SCHEDULER_H_
#define COMET_COMET_CORE_JOB_SCHEDULER_H_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/worker.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/core/essentials.h"
#include "comet/core/type/ring_queue.h"

namespace comet {
namespace job {
namespace internal {
class SchedulerStarterBarrier {
 public:
  SchedulerStarterBarrier() = default;
  SchedulerStarterBarrier(const SchedulerStarterBarrier&) = delete;
  SchedulerStarterBarrier(SchedulerStarterBarrier&&) = delete;
  SchedulerStarterBarrier& operator=(const SchedulerStarterBarrier&) = delete;
  SchedulerStarterBarrier& operator=(SchedulerStarterBarrier&&) = delete;
  ~SchedulerStarterBarrier() = default;

  void Wait();
  void Unleash();
  bool IsReady() const noexcept;

 private:
  bool is_ready_{false};
  std::mutex mutex_{};
  std::condition_variable cv_{};
};
}  // namespace internal

class Scheduler {
 public:
  static Scheduler& Get();

  Scheduler() = default;
  Scheduler(const Scheduler&) = delete;
  Scheduler(Scheduler&&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;
  Scheduler& operator=(Scheduler&&) = delete;
  ~Scheduler() = default;

  void Initialize();
  void Shutdown();
  void Start(const JobDescr& callback_descr);

  Counter* AllocateCounter();
  void FreeCounter(Counter* counter);

  void Kick(const JobDescr& job_descr);
  void Kick(usize job_count, const JobDescr* job_descrs);
  void Kick(const IOJobDescr& job_descr);
  void Kick(usize job_count, const IOJobDescr* job_descrs);

  void Wait(Counter* counter);

  void KickAndWait(const JobDescr& job_descr);
  void KickAndWait(usize job_count, const JobDescr* job_descrs);

 private:
  static constexpr usize kDefaultIOWorkerCount{2};

  usize worker_count_{0};
  usize io_worker_count_{0};
  internal::SchedulerStarterBarrier starter_barrier_{};
  u32 promotion_interval_{1000};

  std::vector<fiber::Fiber*> large_stack_fibers_{};
  std::vector<fiber::Fiber*> gigantic_stack_fibers_{};
  fiber::FiberMutex fiber_pool_mutex_{};

  RingQueue<Counter*> available_counters_{
      COMET_CONF_U16(conf::kCoreJobCounterCount)};
  fiber::SimpleLock counter_lock_{};

  std::vector<Worker> workers_{};
  std::vector<IOWorker> io_workers_{};
  bool is_shutdown_required_{false};

  LockFreeMPMCRingQueue<JobDescr> low_priority_queue_{
      static_cast<usize>(COMET_CONF_U16(conf::kCoreJobQueueCount))};
  LockFreeMPMCRingQueue<JobDescr> normal_priority_queue_{
      static_cast<usize>(COMET_CONF_U16(conf::kCoreJobQueueCount))};
  LockFreeMPMCRingQueue<JobDescr> high_priority_queue_{
      static_cast<usize>(COMET_CONF_U16(conf::kCoreJobQueueCount))};
  LockFreeMPMCRingQueue<IOJobDescr> io_queue_{
      static_cast<usize>(COMET_CONF_U16(conf::kCoreJobQueueCount))};

  void WorkerThread(usize worker_index);
  void Work();
  void IOWorkerThread();
  fiber::Fiber* ResolveFiber(const JobDescr& job_descr);
  static void OnFiberEnd(fiber::Fiber* fiber, void* data);
  void SubmitJob(const JobDescr& job_descr);
  void SubmitJob(const IOJobDescr& job_descr);
  void PromoteJobs();
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_SCHEDULER_H_