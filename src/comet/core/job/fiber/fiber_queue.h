// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_FIBER_QUEUE_H_
#define COMET_COMET_CORE_JOB_FIBER_QUEUE_H_

#include <deque>
#include <mutex>

#include "comet/core/type/primitive.h"
#include "comet/core/job/fiber/fiber.h"

namespace comet {
namespace job {
class FiberQueue {
 public:
  static FiberQueue& Get();

  FiberQueue() = default;
  FiberQueue(const FiberQueue&) = delete;
  FiberQueue(FiberQueue&&) = delete;
  FiberQueue& operator=(const FiberQueue&) = delete;
  FiberQueue& operator=(FiberQueue&&) = delete;
  ~FiberQueue() = default;

  void Enqueue(Fiber* fiber);
  Fiber* Dequeue();
  bool IsEmpty() const noexcept;

 private:
  // TODO(m4jr0): Use lock-free queue.
  mutable std::mutex queue_mutex_{};
  std::deque<Fiber*> queue_{}; // Reserve for N fibers.
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_FIBER_QUEUE_H_