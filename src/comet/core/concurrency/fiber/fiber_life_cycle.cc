// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "fiber_life_cycle.h"

#include <utility>

#include "comet/core/concurrency/fiber/fiber_context.h"

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
               "Destructor called for handler, but it is still initialized!");
}

void FiberLifeCycleHandler::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize handler, but it is already done!");
  queue_allocator_.Initialize();
  is_initialized_ = true;
}

void FiberLifeCycleHandler::Shutdown() {
  COMET_ASSERT(is_initialized_,
               "Tried to shutdown handler, but it is not initialized!");
  queue_allocator_.Destroy();
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

bool FiberLifeCycleHandler::IsInitialized() const noexcept {
  return is_initialized_;
}
}  // namespace fiber
}  // namespace comet