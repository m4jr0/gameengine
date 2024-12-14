// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "fiber_utils.h"

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/frame/frame_manager.h"
#include "comet/entity/entity_manager.h"
#include "comet/time/chrono.h"

namespace comet {
namespace fiber {
void SleepMs(u32 duration_ms) {
  if (duration_ms == 0) {
    return;
  }

  fiber::FiberMutex mutex{};
  fiber::FiberUniqueLock lock{mutex};
  fiber::FiberCV cv{};
  time::Chrono chrono{};
  chrono.Start(duration_ms);
  cv.Wait(lock, [&] { return chrono.IsFinished(); });
}

void WaitForNextFrame() { frame::FrameManager::Get().WaitForNextFrame(); }

void WaitForEntityUpdates() {
  entity::EntityManager::Get().WaitForEntityUpdates();
}
}  // namespace fiber
}  // namespace comet