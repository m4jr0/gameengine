// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "worker.h"

#include "comet/core/concurrency/fiber/fiber_context.h"

namespace comet {
namespace job {
thread_local Worker* tls_current_worker = nullptr;

Worker& Worker::GetCurrent() {
  COMET_ASSERT(tls_current_worker != nullptr,
               "Current worker fiber is null! No fiber has been generated for "
               "this thread!");
  return *tls_current_worker;
}

Worker::Worker(std::thread&& thread)
    : id_{id_counter_++}, thread_{std::move(thread)} {}

Worker::Worker(Worker&& other) noexcept
    : thread_{std::move(other.thread_)},
      worker_fiber_{other.worker_fiber_},
      current_fiber_{other.current_fiber_} {
  other.worker_fiber_ = nullptr;
  other.current_fiber_ = nullptr;
}

Worker& Worker::operator=(Worker&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  thread_ = std::move(other.thread_);
  worker_fiber_ = other.worker_fiber_;
  current_fiber_ = other.current_fiber_;

  other.worker_fiber_ = nullptr;
  other.current_fiber_ = nullptr;
  return *this;
}

void Worker::Initialize() {
  COMET_ASSERT(tls_current_worker == nullptr,
               "A worker has already been assigned to this thread!");
  tls_current_worker = this;
  worker_fiber_ = fiber::ConvertThreadToFiber();
}

void Worker::Destroy() {
  fiber::DestroyFiberFromThread();
  worker_fiber_ = nullptr;

  if (thread_.joinable()) {
    thread_.join();
  }
}

WorkerId Worker::GetId() const noexcept { return id_; }

IOWorker::IOWorker(std::thread&& thread)
    : id_{id_counter_++}, thread_{std::move(thread)} {}

IOWorker::IOWorker(IOWorker&& other) noexcept
    : thread_{std::move(other.thread_)} {}

IOWorker& IOWorker::operator=(IOWorker&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  thread_ = std::move(other.thread_);
  return *this;
}

void IOWorker::Initialize() {}

void IOWorker::Destroy() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

WorkerId IOWorker::GetId() const noexcept { return id_; }
}  // namespace job
}  // namespace comet