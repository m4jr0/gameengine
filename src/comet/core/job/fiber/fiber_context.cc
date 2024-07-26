// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "fiber_context.h"

#include <string>  // >:3

#include "comet/core/debug.h"
#include "comet/core/job/fiber/fiber_primitive.h"
#include "comet/core/job/fiber/fiber_queue.h"
#include "comet/core/job/worker.h"  // >:3 To remove.

namespace comet {
namespace job {
thread_local Fiber tls_thread_fiber{};
thread_local Fiber* tls_current_fiber = nullptr;

extern "C" {
extern void COMET_FORCE_NOT_INLINE
SwitchExecutionContext(ExecutionContext* src, const ExecutionContext* dst);
}

Fiber* GetCurrent() {
  COMET_ASSERT(tls_current_fiber != nullptr, "Current fiber is null!");
  return tls_current_fiber;
}

void Yield() {
  auto* fiber{GetCurrent()};
  auto& fiber_queue{FiberQueue::Get()};

  if (fiber_queue.IsEmpty()) {
    return;
  }

  auto* sleeping_fiber{fiber_queue.Dequeue()};
  FiberQueue::Get().Enqueue(fiber);
  SwitchTo(sleeping_fiber);
}

void SwitchTo(Fiber* to) {
  auto* from{GetCurrent()};
  COMET_ASSERT(from != nullptr, "Fiber to switch from is null!");
  COMET_ASSERT(to != nullptr, "Fiber to switch to is null!");
  tls_current_fiber = to;
  SwitchExecutionContext(&from->GetContext(), &to->GetContext());
}

Fiber* ConvertThreadToFiber() {
  tls_current_fiber = &tls_thread_fiber;
  return &tls_thread_fiber;
}

void DestroyFiberFromThread() { tls_thread_fiber.Destroy(); }
}  // namespace job
}  // namespace comet