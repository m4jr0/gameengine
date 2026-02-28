// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_LIFE_CYCLE_H_
#define COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_LIFE_CYCLE_H_

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/type/ring_queue.h"

namespace comet {
namespace fiber {
namespace internal {
class FiberLifeCycleQueue {
 public:
  FiberLifeCycleQueue() = default;
  FiberLifeCycleQueue(memory::Allocator* allocator, usize capacity);
  FiberLifeCycleQueue(const FiberLifeCycleQueue&) = delete;
  FiberLifeCycleQueue(FiberLifeCycleQueue&& other) noexcept;
  FiberLifeCycleQueue& operator=(const FiberLifeCycleQueue&) = delete;
  FiberLifeCycleQueue& operator=(FiberLifeCycleQueue&& other) noexcept;
  ~FiberLifeCycleQueue() = default;

  void Push(Fiber* fiber);
  Fiber* TryPop();

 private:
  memory::Allocator* allocator_{nullptr};
  RingQueue<Fiber*> queue_{};
};
}  // namespace internal

class FiberLifeCycleHandler {
 public:
  using SleepingFibers = internal::FiberLifeCycleQueue;
  using CompletedFibers = internal::FiberLifeCycleQueue;

  static FiberLifeCycleHandler& Get();

  FiberLifeCycleHandler() = default;
  FiberLifeCycleHandler(const FiberLifeCycleHandler&) = delete;
  FiberLifeCycleHandler(FiberLifeCycleHandler&&) = delete;
  FiberLifeCycleHandler& operator=(const FiberLifeCycleHandler&) = delete;
  FiberLifeCycleHandler& operator=(FiberLifeCycleHandler&&) = delete;
  ~FiberLifeCycleHandler();

  void AttachWorkerFiber(Fiber* fiber);
  void DetachWorkerFiber();
  void PutToSleep(Fiber* fiber);
  Fiber* TryWakingUp();

  void PutToCompleted(Fiber* fiber);
  Fiber* TryGetCompleted();

  bool IsInitialized() const noexcept;

 private:
  // TODO(m4jr0): Use configuration.
  static constexpr usize kQueueCount_{128};
  static inline thread_local Fiber* tls_worker_fiber_{nullptr};

  bool is_initialized_{false};
  // Platform allocator is used because it is only allocated once, during engine
  // startup.
  memory::PlatformAllocator queue_allocator_{memory::kEngineMemoryTagFiber};
  SleepingFibers sleeping_fibers_{&queue_allocator_, kQueueCount_};
  CompletedFibers completed_fibers_{&queue_allocator_, kQueueCount_};
};
}  // namespace fiber
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_LIFE_CYCLE_H_