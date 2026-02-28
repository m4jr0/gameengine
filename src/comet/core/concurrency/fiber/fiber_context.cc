// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "fiber_context.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#ifdef COMET_IS_ASAN
#include <sanitizer/asan_interface.h>
#endif  // COMET_IS_ASAN

#ifdef COMET_IS_TSAN
#include <sanitizer/tsan_interface.h>
#endif  // COMET_IS_TSAN
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_life_cycle.h"
#include "comet/core/concurrency/job/worker_context.h"
#include "comet/core/concurrency/thread/thread_context.h"

namespace comet {
namespace fiber {
static thread_local Fiber tls_thread_fiber{};
static thread_local Fiber* tls_current_fiber{nullptr};

#ifdef COMET_IS_ASAN
static thread_local void* tls_fake_stack{nullptr};
#endif

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
  if (tls_fake_stack == nullptr) {
    __sanitizer_start_switch_fiber(&tls_fake_stack, to->GetStack(),
                                   to->GetStackCapacity());
  }
#endif  //  COMET_IS_ASAN

#ifdef COMET_IS_TSAN
  auto* this_fiber{__tsan_get_current_fiber()};
  auto* next_fiber{__tsan_create_fiber(0)};
  __tsan_switch_to_fiber(next_fiber, 0);
#endif  // COMET_IS_TSAN

  SwitchExecutionContext(&from->GetContext(), &to->GetContext());

#ifdef COMET_IS_ASAN
  const auto* from_stack{from->GetStack()};
  const auto** from_stack_ptr{&from_stack};
  auto from_size{from->GetStackCapacity()};
  __sanitizer_finish_switch_fiber(tls_fake_stack, from_stack_ptr, &from_size);
  tls_fake_stack = nullptr;
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

#ifdef COMET_IS_ASAN
  if (tls_fake_stack == nullptr) {
    __sanitizer_start_switch_fiber(&tls_fake_stack, to->GetStack(),
                                   to->GetStackCapacity());
  }
#endif  //  COMET_IS_ASAN

#ifdef COMET_IS_TSAN
  auto* this_fiber{__tsan_get_current_fiber()};
  auto* next_fiber{__tsan_create_fiber(0)};
  __tsan_switch_to_fiber(next_fiber, 0);
#endif  // COMET_IS_TSAN

  SwitchExecutionContext(&from->GetContext(), &to->GetContext());

#ifdef COMET_IS_ASAN
  const auto* from_stack{from->GetStack()};
  const auto** from_stack_ptr{&from_stack};
  auto from_size{from->GetStackCapacity()};
  __sanitizer_finish_switch_fiber(tls_fake_stack, from_stack_ptr, &from_size);
  tls_fake_stack = nullptr;
#endif  //  COMET_IS_ASAN

#ifdef COMET_IS_TSAN
  __tsan_destroy_fiber(this_fiber);
#endif  // COMET_IS_TSAN
}
}  // namespace internal

bool IsFiber() {
  // Small workaround to force the main thread to behave like a regular thread
  // (non-fiber).
#ifdef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  if (thread::IsMainThread() && job::IsMainThreadWorkerDisabled()) {
    return false;
  }
#endif  // COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  return tls_current_fiber != nullptr;
}

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