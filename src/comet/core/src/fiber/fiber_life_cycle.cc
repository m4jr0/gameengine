// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/fiber/fiber_life_cycle.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace fiber {
namespace internal {
FiberLifeCycleQueue::FiberLifeCycleQueue(memory::Allocator* allocator,
                                         usize capacity)
    : allocator_{allocator} {
  queue_ = RingQueue<Fiber*>{allocator_, capacity};
}

FiberLifeCycleQueue::FiberLifeCycleQueue(FiberLifeCycleQueue&& other) noexcept
    : allocator_{other.allocator_}, queue_{std::move(other.queue_)} {
  other.allocator_ = nullptr;
}

FiberLifeCycleQueue& FiberLifeCycleQueue::operator=(
    FiberLifeCycleQueue&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  allocator_ = other.allocator_;
  queue_ = std::move(other.queue_);

  other.allocator_ = nullptr;
  return *this;
}

void FiberLifeCycleQueue::Push(Fiber* fiber) { queue_.Push(fiber); }

Fiber* FiberLifeCycleQueue::TryPop() {
  if (queue_.IsEmpty()) {
    return nullptr;
  }

  auto* fiber{queue_.Get()};
  queue_.TryPop();
  return fiber;
}
}  // namespace internal

FiberLifeCycleHandler& FiberLifeCycleHandler::Get() {
  static thread_local FiberLifeCycleHandler tls_singleton{};
  return tls_singleton;
}

FiberLifeCycleHandler::~FiberLifeCycleHandler() {
  COMET_ASSERT(!is_initialized_,
               "FiberLifeCycleHandler::~FiberLifeCycleHandler",
               "fiber life cycle handler is still initialized");
  COMET_ASSERT(allocator_ == nullptr,
               "FiberLifeCycleHandler::~FiberLifeCycleHandler",
               "allocator is still attached");
}

void FiberLifeCycleHandler::AttachAllocator(memory::Allocator* allocator,
                                            usize queue_capacity) {
  COMET_ASSERT(!is_initialized_, "FiberLifeCycleHandler::AttachAllocator",
               "fiber life cycle handler is already initialized");
  COMET_ASSERT(allocator_ == nullptr, "FiberLifeCycleHandler::AttachAllocator",
               "allocator is already attached");
  COMET_ASSERT(allocator != nullptr, "FiberLifeCycleHandler::AttachAllocator",
               "allocator is null");
  COMET_ASSERT(queue_capacity > 0, "FiberLifeCycleHandler::AttachAllocator",
               "queue capacity is zero");

  allocator_ = allocator;
  queue_capacity_ = queue_capacity;
}

void FiberLifeCycleHandler::DetachAllocator() {
  COMET_ASSERT(!is_initialized_, "FiberLifeCycleHandler::DetachAllocator",
               "fiber life cycle handler is still initialized");
  COMET_ASSERT(allocator_ != nullptr, "FiberLifeCycleHandler::DetachAllocator",
               "allocator is not attached");

  allocator_ = nullptr;
  queue_capacity_ = 0;
}

void FiberLifeCycleHandler::Initialize() {
  COMET_ASSERT(!is_initialized_, "FiberLifeCycleHandler::Initialize",
               "fiber life cycle handler is already initialized");
  COMET_ASSERT(allocator_ != nullptr, "FiberLifeCycleHandler::Initialize",
               "allocator is not attached");
  COMET_ASSERT(queue_capacity_ > 0, "FiberLifeCycleHandler::Initialize",
               "queue capacity is zero");

  sleeping_fibers_ = SleepingFibers{allocator_, queue_capacity_};
  completed_fibers_ = CompletedFibers{allocator_, queue_capacity_};

  is_initialized_ = true;
}

void FiberLifeCycleHandler::Shutdown() {
  COMET_ASSERT(is_initialized_, "FiberLifeCycleHandler::Shutdown",
               "fiber life cycle handler is not initialized");

  sleeping_fibers_ = SleepingFibers{};
  completed_fibers_ = CompletedFibers{};

  tls_worker_fiber_ = nullptr;
  is_initialized_ = false;
}

void FiberLifeCycleHandler::AttachWorkerFiber(Fiber* fiber) {
  tls_worker_fiber_ = fiber;
}

void FiberLifeCycleHandler::DetachWorkerFiber() { tls_worker_fiber_ = nullptr; }

void FiberLifeCycleHandler::PutToSleep(Fiber* fiber) {
  sleeping_fibers_.Push(fiber);
}

Fiber* FiberLifeCycleHandler::TryWakingUp() {
  return sleeping_fibers_.TryPop();
}

void FiberLifeCycleHandler::PutToCompleted(Fiber* fiber) {
  completed_fibers_.Push(fiber);
}

Fiber* FiberLifeCycleHandler::TryGetCompleted() {
  return completed_fibers_.TryPop();
}
}  // namespace fiber
}  // namespace comet