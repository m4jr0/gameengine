// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "fiber_context.h"

#ifdef COMET_IS_ASAN
#include <sanitizer/asan_interface.h>
#endif  // COMET_IS_ASAN

#ifdef COMET_IS_TSAN
#include <sanitizer/tsan_interface.h>
#endif  // COMET_IS_TSAN

#include "comet/core/concurrency/fiber/fiber_life_cycle.h"

namespace comet {
namespace fiber {
thread_local Fiber tls_thread_fiber{};
thread_local Fiber* tls_current_fiber{nullptr};

extern "C" {
extern void COMET_FORCE_NOT_INLINE
SwitchExecutionContext(ExecutionContext* src, const ExecutionContext* dst);
}

namespace internal {
void SleepTo(Fiber* to) {
  auto* fiber{GetFiber()};
  FiberLifeCycleHandler::Get().PutToSleep(fiber);
  RunOrResume(to);
}

void Sleep(Fiber* fiber) { FiberLifeCycleHandler::Get().PutToSleep(fiber); }

void RunOrResume(Fiber* to) {
  auto* from{GetFiber()};
  COMET_ASSERT(from != nullptr, "Fiber to switch from is null!");
  COMET_ASSERT(to != nullptr, "Fiber to switch to is null!");
  COMET_ASSERT(from != to, "Trying to switch from and to the same fiber!");
  COMET_ASSERT(!IsStackOverflow(), "Stack overflow detected! ",
               from->GetCurrentStackSize(), " > ", from->GetStackCapacity(),
               "!");
  tls_current_fiber = to;
#ifdef COMET_IS_ASAN
  void* fake_stack{nullptr};
  __sanitizer_start_switch_fiber(&fake_stack, to->GetStack(),
                                 to->GetStackCapacity());
#endif  //  COMET_IS_ASAN

#ifdef COMET_IS_TSAN
  auto* this_fiber{__tsan_get_current_fiber()};
  auto* next_fiber{__tsan_create_fiber(0)};
  __tsan_switch_to_fiber(next_fiber, nullptr);
#endif  // COMET_IS_TSAN

  SwitchExecutionContext(&from->GetContext(), &to->GetContext());

#ifdef COMET_IS_ASAN
  __sanitizer_finish_switch_fiber(fake_stack, nullptr, nullptr);
#endif  //  COMET_IS_ASAN

#ifdef COMET_IS_TSAN
  __tsan_destroy_fiber(this_fiber);
#endif  // COMET_IS_TSAN
}

void ResumeWorker() {
  auto* from{GetFiber()};
  auto* to{&tls_thread_fiber};
  COMET_ASSERT(from != nullptr, "Fiber to switch from is null!");
  COMET_ASSERT(to != nullptr, "Fiber to switch to is null!");
  COMET_ASSERT(from != to, "Trying to switch from and to the same fiber!");
  COMET_ASSERT(!IsStackOverflow(), "Stack overflow detected! ",
               from->GetCurrentStackSize(), " > ", from->GetStackCapacity(),
               "!");

  tls_current_fiber = to;
  SwitchExecutionContext(&from->GetContext(), &to->GetContext());
}
}  // namespace internal

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
  if (tls_current_fiber == &tls_thread_fiber) {
    return;
  }

  FiberLifeCycleHandler::Get().PutToSleep(GetFiber());
  internal::ResumeWorker();
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