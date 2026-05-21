// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/job/scheduler_pools.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/fiber/fiber_utils.h"

namespace comet {
namespace job {
namespace internal {
void FiberPool::Initialize(const FiberPoolDescr& descr) {
  COMET_ASSERT(descr.fiber_count > 0, "job::internal::FiberPool::Initialize",
               "fiber count is zero");
  COMET_ASSERT(descr.fiber_stack_size > 0,
               "job::internal::FiberPool::Initialize",
               "fiber stack size is zero");
  COMET_ASSERT(descr.queue_allocator != nullptr,
               "job::internal::FiberPool::Initialize",
               "queue allocator is null");
  COMET_ASSERT(descr.fiber_allocator != nullptr,
               "job::internal::FiberPool::Initialize",
               "fiber allocator is null");

  initial_fiber_count_ = descr.fiber_count;
  fiber_stack_size_ = descr.fiber_stack_size;
  fiber_allocator_ = descr.fiber_allocator;

  fibers_ = LockFreeMPMCRingQueue<fiber::Fiber*>{descr.queue_allocator,
                                                 initial_fiber_count_};

  for (usize i{0}; i < initial_fiber_count_; ++i) {
    auto* fiber{fiber_allocator_->AllocateOneAndPopulate<fiber::Fiber>(
        fiber_stack_size_)};
    fiber->Initialize();
    fibers_.Push(fiber);
  }
}

void FiberPool::Destroy() {
  for (;;) {
    const auto fiber_box{fibers_.TryPop()};

    if (!fiber_box.has_value()) {
      break;
    }

    auto* fiber{fiber_box.value()};
    fiber->Destroy();
    fiber->~Fiber();
  }

  fibers_.Destroy();

  initial_fiber_count_ = 0;
  fiber_stack_size_ = 0;
  fiber_allocator_ = nullptr;
}

fiber::Fiber* FiberPool::TryPop() {
  const auto fiber_box{fibers_.TryPop()};
  fiber::Fiber* fiber{fiber_box.value_or(nullptr)};

  if (fiber != nullptr) {
    fiber->Reset();
  }

  return fiber;
}

void FiberPool::Push(fiber::Fiber* fiber) {
  COMET_ASSERT(fiber != nullptr, "job::internal::FiberPool::Push",
               "fiber is null");
  COMET_ASSERT(fiber->GetStackCapacity() == fiber_stack_size_,
               "job::internal::FiberPool::Push",
               "fiber stack capacity mismatch", "stack_capacity",
               fiber->GetStackCapacity(), "expected_stack_capacity",
               fiber_stack_size_);

  while (!fibers_.TryPush(fiber)) {
    fiber::Yield();
  }
}

usize FiberPool::GetTotalAllocatedStackSize() const {
  return initial_fiber_count_ *
         fiber::Fiber::GetAllocatedStackSize(fiber_stack_size_);
}

void CounterPool::Initialize(const CounterPoolDescr& descr) {
  COMET_ASSERT(descr.counter_count > 0,
               "job::internal::CounterPool::Initialize",
               "counter count is zero");
  COMET_ASSERT(descr.queue_allocator != nullptr,
               "job::internal::CounterPool::Initialize",
               "queue allocator is null");
  COMET_ASSERT(descr.counter_allocator != nullptr,
               "job::internal::CounterPool::Initialize",
               "counter allocator is null");

  counter_allocator_ = descr.counter_allocator;

  counters_ = LockFreeMPMCRingQueue<Counter*>{descr.queue_allocator,
                                              descr.counter_count};

  for (usize i{0}; i < descr.counter_count; ++i) {
    auto* counter{counter_allocator_->AllocateOneAndPopulate<Counter>()};

    while (!counters_.TryPush(counter)) {
      fiber::Yield();
    }
  }
}

void CounterPool::Destroy() {
  for (;;) {
    const auto counter_box{counters_.TryPop()};

    if (!counter_box.has_value()) {
      break;
    }

    auto* counter{counter_box.value()};
    counter->~Counter();
  }

  counters_.Destroy();
  counter_allocator_ = nullptr;
}

Counter* CounterPool::TryGet() {
  const auto counter_box{counters_.TryPop()};
  return counter_box.value_or(nullptr);
}

void CounterPool::Push(Counter* counter) {
  COMET_ASSERT(counter != nullptr, "job::internal::CounterPool::Push",
               "counter is null");

  while (!counters_.TryPush(counter)) {
    fiber::Yield();
  }
}
}  // namespace internal
}  // namespace job
}  // namespace comet