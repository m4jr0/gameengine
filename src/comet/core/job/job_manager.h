// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_JOB_MANAGER_H_
#define COMET_COMET_CORE_JOB_JOB_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/job/job.h"
#include "comet/core/manager.h"

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

  Counter* AllocateCounter();
  void FreeCounter(Counter* counter);

  void Kick(const JobDescr& job_descr);
  void Kick(uindex job_count, const JobDescr* job_descrs);

  void Wait(Counter* counter);

  void KickAndWait(const JobDescr& job_descr);
  void KickAndWait(uindex job_count, const JobDescr* job_descrs);
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_JOB_MANAGER_H_