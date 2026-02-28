// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "worker.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_context.h"
#include "comet/core/concurrency/fiber/fiber_life_cycle.h"
#include "comet/core/concurrency/job/worker_context.h"
#include "comet/core/frame/frame_allocator.h"
#include "comet/core/frame/frame_manager.h"
#include "comet/core/type/tstring.h"

namespace comet {
namespace job {
void Worker::Attach() {
  internal::AttachWorker(this);
  frame::AttachFrameAllocator(frame::FrameManager::Get().GetFrameAllocator());
  frame::AttachDoubleFrameAllocator(
      frame::FrameManager::Get().GetDoubleFrameAllocator());
}

void Worker::Detach() {
  internal::DetachWorker();
  frame::DetachFrameAllocator();
  frame::DetachDoubleFrameAllocator();
}

void Worker::Stop() { thread_.TryJoin(); }

WorkerId Worker::GetId() const noexcept {
  return (static_cast<WorkerId>(tag_) << 32) + static_cast<u32>(type_index_);
}

WorkerTag Worker::GetTag() const noexcept { return tag_; }

WorkerTypeIndex Worker::GetTypeIndex() const noexcept { return type_index_; }

Worker::Worker(WorkerTag tag, WorkerTypeIndex type_index)
    : tag_{tag}, type_index_{type_index} {}

Worker::Worker(Worker&& other) noexcept
    : tag_{other.tag_},
      type_index_{other.type_index_},
      thread_{std::move(other.thread_)} {
  other.type_index_ = kInvalidWorkerTypeIndex;
  other.tag_ = kInvalidWorkerTag;
}

Worker& Worker::operator=(Worker&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  tag_ = other.tag_;
  type_index_ = other.type_index_;
  thread_ = std::move(other.thread_);

  other.type_index_ = kInvalidWorkerTypeIndex;
  other.tag_ = kInvalidWorkerTag;
  return *this;
}

FiberWorker::FiberWorker()
    : Worker{kTag_, index_counter_.fetch_add(1, std::memory_order_acq_rel)} {}

FiberWorker::FiberWorker(FiberWorker&& other) noexcept
    : Worker{std::move(other)},
      worker_fiber_{other.worker_fiber_},
      current_fiber_{other.current_fiber_} {
  other.worker_fiber_ = nullptr;
  other.current_fiber_ = nullptr;
}

FiberWorker& FiberWorker::operator=(FiberWorker&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Worker::operator=(std::move(other));
  worker_fiber_ = other.worker_fiber_;
  current_fiber_ = other.current_fiber_;
  other.worker_fiber_ = nullptr;
  other.current_fiber_ = nullptr;
  return *this;
}

void FiberWorker::Attach() {
  Worker::Attach();
  internal::AttachFiberWorker(this);
  worker_fiber_ = fiber::ConvertThreadToFiber();
  fiber::FiberLifeCycleHandler::Get().AttachWorkerFiber(worker_fiber_);
}

void FiberWorker::Detach() {
  Worker::Detach();
  fiber::DestroyFiberFromThread();
  fiber::FiberLifeCycleHandler::Get().DetachWorkerFiber();
  worker_fiber_ = nullptr;
  internal::DetachFiberWorker();
}

IOWorker::IOWorker()
    : Worker{kTag_, index_counter_.fetch_add(1, std::memory_order_acq_rel)} {}

IOWorker::IOWorker(IOWorker&& other) noexcept : Worker{std::move(other)} {}

IOWorker& IOWorker::operator=(IOWorker&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Worker::operator=(std::move(other));
  return *this;
}

void IOWorker::Attach() {
  Worker::Attach();
  AttachTStringAllocator(frame::FrameManager::Get().GetFrameAllocator());
  internal::AttachIOWorker(this);
}

void IOWorker::Detach() {
  Worker::Detach();
  DetachTStringAllocator();
  internal::DetachIOWorker();
}
}  // namespace job
}  // namespace comet