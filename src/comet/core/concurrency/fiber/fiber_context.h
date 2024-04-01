// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_CONTEXT_H_
#define COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_CONTEXT_H_

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/essentials.h"

namespace comet {
namespace fiber {
bool IsFiber();
Fiber* GetCurrent();
void Yield();
void SwitchTo(Fiber* to);
Fiber* ConvertThreadToFiber();
void DestroyFiberFromThread();
}  // namespace fiber
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_CONTEXT_H_