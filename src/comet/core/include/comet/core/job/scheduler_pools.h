// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_JOB_SCHEDULER_POOLS_H_
#define COMET_CORE_JOB_SCHEDULER_POOLS_H_

#include "comet/core/essentials.h"
#include "comet/core/fiber/fiber.h"
#include "comet/core/job/job.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/ring_queue.h"

namespace comet {
namespace job {
namespace internal {
struct FiberPoolDescr {
  usize fiber_count{0};
  usize fiber_stack_size{0};
  memory::Allocator* queue_allocator{nullptr};
  memory::Allocator* fiber_allocator{nullptr};
};

class FiberPool {
 public:
  FiberPool() = default;
  FiberPool(const FiberPool&) = delete;
  FiberPool(FiberPool&&) = delete;
  FiberPool& operator=(const FiberPool&) = delete;
  FiberPool& operator=(FiberPool&&) = delete;
  ~FiberPool() = default;

  void Initialize(const FiberPoolDescr& descr);
  void Destroy();

  fiber::Fiber* TryPop();
  void Push(fiber::Fiber* fiber);

  usize GetTotalAllocatedStackSize() const;

 private:
  usize initial_fiber_count_{0};
  usize fiber_stack_size_{0};
  memory::Allocator* fiber_allocator_{nullptr};
  LockFreeMPMCRingQueue<fiber::Fiber*> fibers_{};
};

struct CounterPoolDescr {
  usize counter_count{0};
  memory::Allocator* queue_allocator{nullptr};
  memory::Allocator* counter_allocator{nullptr};
};

class CounterPool {
 public:
  CounterPool() = default;
  CounterPool(const CounterPool&) = delete;
  CounterPool(CounterPool&&) = delete;
  CounterPool& operator=(const CounterPool&) = delete;
  CounterPool& operator=(CounterPool&&) = delete;
  ~CounterPool() = default;

  void Initialize(const CounterPoolDescr& descr);
  void Destroy();

  Counter* TryGet();
  void Push(Counter* counter);

 private:
  memory::Allocator* counter_allocator_{nullptr};
  LockFreeMPMCRingQueue<Counter*> counters_{};
};
}  // namespace internal
}  // namespace job
}  // namespace comet

#endif  // COMET_CORE_JOB_SCHEDULER_POOLS_H_