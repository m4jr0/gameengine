// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "fiber_queue.h"

namespace comet {
namespace fiber {
FiberQueue& FiberQueue::Get() {
  static FiberQueue singleton{};
  return singleton;
}

void FiberQueue::Push(Fiber* fiber) { queue_.Push(fiber); }

Fiber* FiberQueue::Pop() {
  auto fiber{queue_.Pop()};
  return fiber.has_value() ? fiber.value() : nullptr;
}
}  // namespace fiber
}  // namespace comet