// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "fiber_primitive.h"

namespace comet {
namespace job {
void FiberMutex::Lock() { auto* current{GetCurrentFiber()};
  while (is_locked_.exchange(true, std::memory_order_acquire)) {
    awaiting_fibers_.push_back(current);
    // >:3 ????
  }
}

void FiberMutex::Unlock() {}

void FiberCV::Wait(FiberMutex& mtx) {
  mtx.Unlock();



  mtx.Lock();
}

void FiberCV::NotifyOne() {}

void FiberCV::NotifyAll() {}
}  // namespace job
}  // namespace comet