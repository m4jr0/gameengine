// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_SCHEDULER_H_
#define COMET_COMET_CORE_JOB_SCHEDULER_H_

#include <mutex>
#include <queue>
#include <vector>

#include "comet/core/job/fiber/fiber.h"
#include "comet/core/job/fiber/fiber_primitive.h"
#include "comet/core/job/job.h"
#include "comet/core/job/worker.h"
#include "comet/core/type/primitive.h"
#include "comet/core/type/ring_queue.h"

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
  virtual ~Scheduler() = default;

  void Initialize();
  void Shutdown();

  Counter* AllocateCounter();
  void FreeCounter(Counter* counter);

  void Kick(const JobDescr& job_descr);
  void Kick(uindex job_count, const JobDescr* job_descrs);
  void Kick(const IOJobDescr& job_descr);
  void Kick(uindex job_count, const IOJobDescr* job_descrs);

  void Wait(Counter* counter);

  void KickAndWait(const JobDescr& job_descr);
  void KickAndWait(uindex job_count, const JobDescr* job_descrs);

 private:
  // TODO(m4jr0): Use configuration.
  const uindex kLargeStackFiberCount_{128};
  const uindex kGiganticStackFiberCount_{32};

  std::vector<Fiber*> large_stack_fibers_{};
  std::vector<Fiber*> gigantic_stack_fibers_{};
  FiberMutex fiber_pool_mutex_{};

  const uindex kCounterCount_{256};
  Counter* counters_{nullptr};
  ring_queue<Counter*> available_counters_{0};
  SimpleLock counter_lock_{};

  std::vector<Worker> workers_{};
  std::vector<IOWorker> io_workers_{};
  std::atomic<uindex> worker_ready_count_{0};
  bool is_shutdown_required_{false};

  // >:3 Preallocate.
  std::queue<JobDescr> low_priority_queue_{};
  std::queue<JobDescr> normal_priority_queue_{};
  std::queue<JobDescr> high_priority_queue_{};
  std::queue<IOJobDescr> io_queue_{};
  std::mutex sleeping_fibers_mutex_{};
  std::queue<Fiber*> sleeping_fibers_queue_{};
  SimpleLock queue_lock_{};
  SimpleLock io_queue_lock_{};
  std::mutex io_mutex_{};
  std::condition_variable io_cv_{};

  f64 last_promotion_time_{0};
  f64 promotion_interval_{1000};

  void WorkerThread(uindex worker_index);
  void IOWorkerThread();
  Fiber* ResolveFiber(const JobDescr& job_descr);
  static void OnFiberEnd(Fiber* fiber);
  void SubmitJob(const JobDescr& job_descr);
  void SubmitJob(const IOJobDescr& job_descr);
  void PromoteJobs();
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_SCHEDULER_H_