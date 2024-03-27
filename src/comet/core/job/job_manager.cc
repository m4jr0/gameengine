// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "job_manager.h"

#include "comet/core/job/fiber/fiber.h"

namespace comet {
namespace job {
JobManager& JobManager::Get() {
  static JobManager singleton{};

  TestFibers();

  return singleton;
}

Counter* JobManager::AllocateCounter() { return nullptr; }

void JobManager::FreeCounter(Counter* counter) {}

void JobManager::Kick(const JobDescr& job_descr) {}

void JobManager::Kick(uindex job_count, const JobDescr* job_descrs) {}

void JobManager::Wait(Counter* counter) {}

void JobManager::KickAndWait(const JobDescr& job_descr) {}

void JobManager::KickAndWait(uindex job_count, const JobDescr* job_descrs) {}
}  // namespace job
}  // namespace comet