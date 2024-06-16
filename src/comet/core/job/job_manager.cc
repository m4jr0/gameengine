// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "job_manager.h"

#include "comet/core/job/fiber/fiber.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace job {
JobManager& JobManager::Get() {
  static JobManager singleton{};

  // TestFibers();

  return singleton;
}

void JobManager::Initialize() {
  large_stack_fibers_.reserve(kLargeStackFiberCount_);

  for (uindex i{0}; i < kLargeStackFiberCount_; ++i) {
    // Update code.
    large_stack_fibers_.emplace_back(Generate(kLargeStack, nullptr, nullptr));
  }

  gigantic_stack_fibers_.reserve(kGiganticStackFiberCount_);

  for (uindex i{0}; i < kGiganticStackFiberCount_; ++i) {
    // Update code.
    gigantic_stack_fibers_.emplace_back(
        Generate(kGiganticStack, nullptr, nullptr));
  }

  uindex worker_count{std::thread::hardware_concurrency()};
  workers_.reserve(worker_count);

  for (uindex i{0}; i < worker_count; ++i) {
    workers_.emplace_back(std::thread{&JobManager::WorkerThread, this}, nullptr);
  }
}

void JobManager::Shutdown() {
  is_shutdown_required_ = true;

  for (uindex i{0}; i < large_stack_fibers_.size(); ++i) {
    auto* fiber{large_stack_fibers_[i]};

    if (fiber == nullptr) {
      continue;
    }

    Destroy(fiber);
  }

  large_stack_fibers_.clear();

  for (uindex i{0}; i < gigantic_stack_fibers_.size(); ++i) {
    auto* fiber{gigantic_stack_fibers_[i]};

    if (fiber == nullptr) {
      continue;
    }

    Destroy(fiber);
  }

  gigantic_stack_fibers_.clear();
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

thread_local Worker* tls_worker = nullptr;

void JobManager::WorkerThread() {
  COMET_ASSERT(tls_worker == nullptr,
               "A worker has already been assigned to this thread!");

  while (!is_shutdown_required_) {
    auto is_job{false};
    JobDescr job{0};
    auto current_time{time::TimeManager::Get().GetCurrentTime()};

    if (current_time - last_promotion_time_ >= promotion_interval_) {
      PromoteJobs();
      last_promotion_time_ = current_time;
    }

    if (!critical_priority_queue_.empty()) {
      job = critical_priority_queue_.front();
      critical_priority_queue_.pop();
      is_job = true;
    } else if (!high_priority_queue_.empty()) {
      job = high_priority_queue_.front();
      high_priority_queue_.pop();
      is_job = true;
    } else if (!normal_priority_queue_.empty()) {
      job = normal_priority_queue_.front();
      normal_priority_queue_.pop();
      is_job = true;
    } else if (!low_priority_queue_.empty()) {
      job = low_priority_queue_.front();
      low_priority_queue_.pop();
      is_job = true;
    }

    if (!is_job) {
      available_job_mutex_.Lock();
      available_job_cv_.Wait(available_job_mutex_);
    }
  }
}

void JobManager::SubmitJob(const JobDescr& job_descr) {}

void JobManager::PromoteJobs() {}

Fiber* JobManager::GetFiber(const EntryPoint& entry_point) { return nullptr; }
}  // namespace job
}  // namespace comet