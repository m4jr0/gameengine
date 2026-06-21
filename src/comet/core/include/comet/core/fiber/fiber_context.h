// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_FIBER_FIBER_CONTEXT_H_
#define COMET_CORE_FIBER_FIBER_CONTEXT_H_

#include "comet/core/fiber/fiber.h"
#include "comet/core/essentials.h"

namespace comet {
namespace fiber {
namespace internal {
void SleepTo(Fiber* to);
void Sleep(Fiber* fiber);
void RunOrResume(Fiber* to);
void ResumeWorker();
}  // namespace internal

bool IsFiber();
FiberId GetFiberId();
Fiber* GetFiber();
void Yield();
Fiber* ConvertThreadToFiber();
void DestroyFiberFromThread();
usize GetStackCapacity();
usize GetCurrentStackSize();
usize GetCurrentStackSizeLeft();
bool IsStackOverflow();
}  // namespace fiber
}  // namespace comet

#ifndef COMET_DEBUG
#define COMET_LOG_WITH_FIBER_ID(FIBER_ID, LOG_FUNC, ...)
#define COMET_LOG_WITH_FIBER_DEBUG_LABEL(FIBER_DEBUG_LABEL, LOG_FUNC, ...)
#else
#define COMET_LOG_WITH_FIBER_ID(FIBER_ID, LOG_FUNC, ...) \
  do {                                                   \
    if (comet::fiber::GetFiberId() == FIBER_ID) {        \
      LOG_FUNC(__VA_ARGS__);                             \
    }                                                    \
  } while (false)

#ifdef COMET_FIBER_DEBUG_LABEL
#define COMET_LOG_WITH_FIBER_DEBUG_LABEL(FIBER_DEBUG_LABEL, LOG_FUNC, ...) \
  do {                                                                     \
    if (AreStringsEqual(comet::fiber::GetFiber()->GetDebugLabel(),         \
                        FIBER_DEBUG_LABEL)) {                              \
      LOG_FUNC(__VA_ARGS__);                                               \
    }                                                                      \
  } while (false)
#else
#define COMET_LOG_WITH_FIBER_DEBUG_LABEL(FIBER_DEBUG_LABEL, LOG_FUNC, ...)
#endif  // COMET_FIBER_DEBUG_LABEL
#endif  // !COMET_DEBUG

#endif  // COMET_CORE_FIBER_FIBER_CONTEXT_H_