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

void JobManager::FreeCounter([[maybe_unused]] Counter* counter) {}

void JobManager::Kick([[maybe_unused]] const JobDescr& job_descr) {}

void JobManager::Kick([[maybe_unused]] uindex job_count,
                      [[maybe_unused]] const JobDescr* job_descrs) {}

void JobManager::Wait([[maybe_unused]] Counter* counter) {}

void JobManager::KickAndWait([[maybe_unused]] const JobDescr& job_descr) {}

void JobManager::KickAndWait([[maybe_unused]] uindex job_count,
                             [[maybe_unused]] const JobDescr* job_descrs) {}
}  // namespace job
}  // namespace comet