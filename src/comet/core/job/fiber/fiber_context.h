// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_FIBER_CONTEXT_H_
#define COMET_COMET_CORE_JOB_FIBER_CONTEXT_H_

#include "comet/core/job/fiber/fiber.h"

namespace comet {
namespace job {
bool IsFiber();
Fiber* GetCurrent();
void Yield();
void SwitchTo(Fiber* to);
Fiber* ConvertThreadToFiber();
void DestroyFiberFromThread();
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_FIBER_CONTEXT_H_