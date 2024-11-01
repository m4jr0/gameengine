// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "fiber_context.h"

#include "comet/core/concurrency/fiber/fiber_life_cycle.h"
#include "comet/core/logger.h"

namespace comet {
namespace fiber {
thread_local Fiber tls_thread_fiber{};
thread_local Fiber* tls_current_fiber{nullptr};

extern "C" {
extern void COMET_FORCE_NOT_INLINE
SwitchExecutionContext(ExecutionContext* src, const ExecutionContext* dst);
}

bool IsFiber() { return tls_current_fiber != nullptr; }

FiberId GetFiberId() {
  return tls_current_fiber != nullptr ? tls_current_fiber->GetId()
                                      : kInvalidFiberId;
}

Fiber* GetFiber() {
  COMET_ASSERT(tls_current_fiber != nullptr, "Current fiber is null!");
  return tls_current_fiber;
}

void Yield() {
  auto* fiber{GetFiber()};
  auto& life_cycle_handler{FiberLifeCycleHandler::Get()};
  auto* sleeping_fiber{life_cycle_handler.TryWakingUp()};

  if (sleeping_fiber == nullptr) {
    return;
  }

  life_cycle_handler.PutToSleep(fiber);
  SwitchTo(sleeping_fiber);
}

void SwitchTo(Fiber* to) {
  auto* from{GetFiber()};
  COMET_ASSERT(from != nullptr, "Fiber to switch from is null!");
  COMET_ASSERT(to != nullptr, "Fiber to switch to is null!");
  COMET_ASSERT(!IsStackOverflow(), "Stack overflow detected! ",
               from->GetCurrentStackSize(), " > ", from->GetStackCapacity(),
               "!");
  tls_current_fiber = to;
  SwitchExecutionContext(&from->GetContext(), &to->GetContext());
}

Fiber* ConvertThreadToFiber() {
  tls_current_fiber = &tls_thread_fiber;
  return &tls_thread_fiber;
}

void DestroyFiberFromThread() { tls_thread_fiber.Destroy(); }

usize GetStackCapacity() {
  auto* fiber{GetFiber()};
  COMET_ASSERT(fiber != nullptr, "Current fiber is null!");
  return fiber->GetStackCapacity();
}

usize GetCurrentStackSize() {
  auto* fiber{GetFiber()};
  COMET_ASSERT(fiber != nullptr, "Current fiber is null!");
  return fiber->GetCurrentStackSize();
}

usize GetCurrentStackSizeLeft() {
  auto* fiber{GetFiber()};
  COMET_ASSERT(fiber != nullptr, "Current fiber is null!");
  return fiber->GetCurrentStackSizeLeft();
}

bool IsStackOverflow() {
  auto* fiber{GetFiber()};
  COMET_ASSERT(fiber != nullptr, "Current fiber is null!");
  return fiber->IsStackOverflow();
}
}  // namespace fiber
}  // namespace comet