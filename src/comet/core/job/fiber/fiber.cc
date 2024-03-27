// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "fiber.h"

#include <winternl.h>

#include "comet/core/memory/memory.h"

// TODO(m4jr0): Support other architectures.
#ifndef COMET_ARCH_X86_64
static_assert(false, "Unsupported architecture.");
#endif  // !COMET_ARCH_X86_64

namespace comet {
namespace job {

struct TestHere {
  PVOID stack_base;
  PVOID stack_limit;
};

TestHere here{};

void SaveThreadEnvironment() {
  return;
  auto* tib{reinterpret_cast<NT_TIB*>(NtCurrentTeb())};
  here.stack_base = tib->StackBase;
  here.stack_limit = tib->StackLimit;
}

void RestoreThreadEnvironment() {
  return;
  auto* tib{reinterpret_cast<NT_TIB*>(NtCurrentTeb())};
  tib->StackBase = here.stack_base;
  tib->StackLimit = here.stack_limit;
}

void SetThreadEnvironment(const Fiber* fiber) {
  return;
  auto* tib{reinterpret_cast<NT_TIB*>(NtCurrentTeb())};
  tib->StackBase = reinterpret_cast<PVOID>(fiber->stack + fiber->stack_size);
  tib->StackLimit = reinterpret_cast<PVOID>(fiber->stack);
}

extern "C" void SwitchExecutionContext(ExecutionContext* src,
                                       const ExecutionContext* dst);

Fiber* Generate(uindex stack_size, void* func, void* data) {
  COMET_ASSERT(stack_size != 0, "Stack size provided is 0!");
  COMET_ASSERT(func != nullptr, "Function provided is null!");

  auto* fiber{AllocateAligned<Fiber>(MemoryTag::Fiber)};

#ifdef COMET_MSVC
  constexpr auto kRedZoneOffset{0};
#else
  constexpr auto kRedZoneOffset{128};
#endif  // COMET_MSVC

  fiber->stack = static_cast<u8*>(AllocateAligned(
      stack_size + kRedZoneOffset, kStackAlignment, MemoryTag::Fiber));
  fiber->stack_size = stack_size;
  fiber->func = func;

  auto& context{fiber->context};
  context.rip = reinterpret_cast<RegisterValue>(fiber->func);
  context.rsp =
      reinterpret_cast<RegisterValue>(fiber->stack + fiber->stack_size);
#ifdef COMET_MSVC
  context.rcx = reinterpret_cast<RegisterValue>(data);
#else
  context.rdi = reinterpret_cast<RegisterValue>(data);
#endif  // COMET_MSVC
  return fiber;
}

void Destroy(Fiber* fiber) {
  COMET_ASSERT(fiber != nullptr, "Fiber provided is null!");
  SetThreadEnvironment(fiber);

  if (fiber->stack != nullptr) {
    Deallocate(fiber->stack);
    fiber->stack = nullptr;
  }

  Deallocate(fiber);
  RestoreThreadEnvironment();
}

void Switch(Fiber* from, Fiber* to) {
  COMET_ASSERT(from != nullptr, "Fiber to switch from is null!");
  COMET_ASSERT(to != nullptr, "Fiber to switch to is null!");

  SetThreadEnvironment(to);
  SwitchExecutionContext(&from->context, &to->context);
}

Fiber* SwitchThreadToFiber() {
  auto* fiber{AllocateAligned<Fiber>(MemoryTag::Fiber)};
  fiber->stack = nullptr;
  fiber->stack_size = 0;
  return fiber;

  auto* tib{reinterpret_cast<NT_TIB*>(NtCurrentTeb())};
  fiber->stack = reinterpret_cast<u8*>(tib->StackBase);
  auto test123{(uptr)tib->StackBase - (uptr)tib->StackLimit};
  fiber->stack_size = static_cast<uindex>(test123);
  return fiber;
}

Fiber* main;
Fiber* fiber_a;
Fiber* fiber_b;
ExecutionContext* current_ctx;

struct CustomData {
  s32 a{1};
  f64 b{2.0f};
};

void ComputeA(void* data) {
  auto* custom_data{static_cast<CustomData*>(data)};
  s32 a{0};
  ++a;

  Switch(fiber_a, main);

  ++a;

  Switch(fiber_a, main);
}

void ComputeB(void* data) {
  s32 a{1337};
  ++a;

  Switch(fiber_b, main);
}

void FiberFunc(LPVOID lpParameter) {
  auto* tib = reinterpret_cast<NT_TIB*>(NtCurrentTeb());
  printf("Inside fiber\n");
  SwitchToFiber(lpParameter);  // Switch back to the main fiber
}

void TestFibers() {
  //auto* tib = reinterpret_cast<NT_TIB*>(NtCurrentTeb());
  //LPVOID mainFiber =
  //    ConvertThreadToFiber(NULL);  // Convert main thread to fiber
  //LPVOID fiber = CreateFiber(0, FiberFunc, mainFiber);  // Create a new fiber

  //printf("Switching to fiber\n");
  //SwitchToFiber(fiber);  // Switch to the new fiber

  //printf("Back to main fiber\n");

  //DeleteFiber(fiber); 


  //tib = reinterpret_cast<NT_TIB*>(NtCurrentTeb());
  SaveThreadEnvironment();
  main = SwitchThreadToFiber();
  CustomData data{42, 1337.0f};
  fiber_a = Generate(1024, ComputeA, &data);
  fiber_b = Generate(1024, ComputeB);

  Switch(main, fiber_a);
  Switch(main, fiber_b);

  s32 a{42};
  ++a;

  Switch(main, fiber_a);

  ++a;

  Destroy(fiber_a);
  Destroy(fiber_b);
  Destroy(main);
}
}  // namespace job
}  // namespace comet