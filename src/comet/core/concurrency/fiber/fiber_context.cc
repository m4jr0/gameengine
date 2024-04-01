// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "fiber_context.h"

#include "comet/core/concurrency/fiber/fiber_queue.h"

namespace comet {
namespace fiber {
thread_local Fiber tls_thread_fiber{};
thread_local Fiber* tls_current_fiber = nullptr;

extern "C" {
extern void COMET_FORCE_NOT_INLINE
SwitchExecutionContext(ExecutionContext* src, const ExecutionContext* dst);
}

bool IsFiber() { return tls_current_fiber != nullptr; }

Fiber* GetCurrent() {
  COMET_ASSERT(tls_current_fiber != nullptr, "Current fiber is null!");
  return tls_current_fiber;
}

void Yield() {
  auto* fiber{GetCurrent()};
  auto& fiber_queue{FiberQueue::Get()};
  auto* sleeping_fiber{fiber_queue.Pop()};

  if (sleeping_fiber == nullptr) {
    return;
  }

  fiber_queue.Push(fiber);
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
}  // namespace fiber
}  // namespace comet