// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_QUEUE_H_
#define COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_QUEUE_H_

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/essentials.h"
#include "comet/core/type/ring_queue.h"

namespace comet {
namespace fiber {
class FiberQueue {
 public:
  static FiberQueue& Get();

  FiberQueue() = default;
  FiberQueue(const FiberQueue&) = delete;
  FiberQueue(FiberQueue&&) = delete;
  FiberQueue& operator=(const FiberQueue&) = delete;
  FiberQueue& operator=(FiberQueue&&) = delete;
  ~FiberQueue() = default;

  void Push(Fiber* fiber);
  Fiber* Pop();

 private:
  // TODO(m4jr0): Use configuration.
  static constexpr usize kQueueCount_{128};

  RingQueue<Fiber*> queue_{kQueueCount_};
};
}  // namespace fiber
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_QUEUE_H_