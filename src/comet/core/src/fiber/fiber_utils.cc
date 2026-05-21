// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/fiber/fiber_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/time/chrono.h"

namespace comet {
namespace fiber {
void SleepMs(u32 duration_ms) {
  if (duration_ms == 0) {
    return;
  }

  FiberMutex mutex{};
  FiberUniqueLock lock{mutex};
  time::Chrono chrono{};
  chrono.Start(duration_ms);

  while (!chrono.IsFinished()) {
    Yield();
  }
}
}  // namespace fiber
}  // namespace comet