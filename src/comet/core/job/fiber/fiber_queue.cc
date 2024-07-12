// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "fiber_queue.h"

#include "comet/core/debug.h"

namespace comet {
namespace job {
FiberQueue& FiberQueue::Get() {
  static thread_local FiberQueue singleton{};
  return singleton;
}

void FiberQueue::Enqueue(Fiber* fiber) {
  std::unique_lock lock{queue_mutex_};
  queue_.emplace_back(fiber);
}

Fiber* FiberQueue::Dequeue() {
  std::unique_lock lock{queue_mutex_};
  auto* fiber{queue_.front()};
  queue_.pop_front();
  return fiber;
}

bool FiberQueue::IsEmpty() const noexcept {
  std::unique_lock lock{queue_mutex_};
  return queue_.empty();
}
}  // namespace job
}  // namespace comet