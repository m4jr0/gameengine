// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_JOB_MANAGER_H_
#define COMET_COMET_CORE_JOB_JOB_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/job/job.h"
#include "comet/core/manager.h"
#include "comet/core/job/fiber/fiber.h"
#include "comet/core/job/fiber/fiber_primitive.h"
#include "comet/core/job/worker.h"

namespace comet {
namespace job {
class JobManager : public Manager {
 public:
  static JobManager& Get();

  JobManager() = default;
  JobManager(const JobManager&) = delete;
  JobManager(JobManager&&) = delete;
  JobManager& operator=(const JobManager&) = delete;
  JobManager& operator=(JobManager&&) = delete;
  virtual ~JobManager() = default;

  void Initialize() override;
  void Shutdown() override;

  Counter* AllocateCounter();
  void FreeCounter(Counter* counter);

  void Kick(const JobDescr& job_descr);
  void Kick(uindex job_count, const JobDescr* job_descrs);

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

  std::vector<Worker> workers_{};
  bool is_shutdown_required_{false};

  // >:3 Preallocate.
  std::queue<JobDescr> low_priority_queue_{};
  std::queue<JobDescr> normal_priority_queue_{};
  std::queue<JobDescr> high_priority_queue_{};
  std::queue<JobDescr> critical_priority_queue_{};
  FiberMutex queue_mutex_{};
  FiberCV available_job_cv_{};
  FiberMutex available_job_mutex_{};

  f64 last_promotion_time_{0};
  f64 promotion_interval_{1000};

  void WorkerThread();
  void SubmitJob(const JobDescr& job_descr);
  void PromoteJobs();
  Fiber* GetFiber(const EntryPoint& entry_point);
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_JOB_MANAGER_H_