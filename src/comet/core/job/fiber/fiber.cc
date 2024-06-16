// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "fiber.h"

#include "comet/core/memory/memory.h"

// TODO(m4jr0): Support other architectures.
#ifndef COMET_ARCH_X86_64
static_assert(false, "Unsupported architecture.");
#endif  // !COMET_ARCH_X86_64

namespace comet {
namespace job {
extern "C" {
extern void COMET_FORCE_NOT_INLINE
SwitchExecutionContext(ExecutionContext* src, const ExecutionContext* dst);
}

Fiber* Generate(uindex stack_size, FiberFunc func, SwitchData* data) {
  COMET_ASSERT(stack_size != 0, "Stack size provided is 0!");
  COMET_ASSERT(func != nullptr, "Function provided is null!");

  auto* fiber{AllocateAligned<Fiber>(MemoryTag::Fiber)};
  fiber->context = {};

#ifdef COMET_MSVC
  constexpr auto kRedZoneOffset{0};
#else
  constexpr auto kRedZoneOffset{128};
#endif  // COMET_MSVC

  fiber->stack =
      static_cast<u8*>(Allocate(stack_size + kRedZoneOffset, MemoryTag::Fiber));
  fiber->stack_size = stack_size;
  fiber->func = func;

  auto& context{fiber->context};
  context.rip = reinterpret_cast<RegisterValue>(fiber->func);

  // On supported architectures, stack grows downwards.
  auto* stack_top{reinterpret_cast<uptr*>(fiber->stack + fiber->stack_size)};

  // Align stack before setting RSP.
  stack_top = reinterpret_cast<uptr*>(
      reinterpret_cast<u8*>(
          (reinterpret_cast<uptr>(&stack_top[-1]) & -kStackAlignment)) -
      kStackCallingOffset);

  *stack_top = 0x0;  // Set return address to 0.
  context.rsp = reinterpret_cast<RegisterValue>(stack_top);

#ifdef COMET_MSVC
  context.rcx = reinterpret_cast<RegisterValue>(data);
#else
  context.rdi = reinterpret_cast<RegisterValue>(data);
#endif  // COMET_MSVC
  return fiber;
}

void Destroy(Fiber* fiber) {
  COMET_ASSERT(fiber != nullptr, "Fiber provided is null!");

  if (fiber->stack != nullptr) {
    Deallocate(fiber->stack);
    fiber->stack = nullptr;
  }

  Deallocate(fiber);
}

void Switch(Fiber* from, Fiber* to) {
  COMET_ASSERT(from != nullptr, "Fiber to switch from is null!");
  COMET_ASSERT(to != nullptr, "Fiber to switch to is null!");
  tls_current_fiber = to;
  SwitchExecutionContext(&from->context, &to->context);
}

Fiber* SwitchThreadToFiber() {
  auto* fiber{AllocateAligned<Fiber>(MemoryTag::Fiber)};
  fiber->context = {};
  fiber->stack = nullptr;
  fiber->stack_size = 0;
  return fiber;
}

thread_local Fiber* tls_current_fiber = nullptr;

Fiber* GetCurrentFiber() { return tls_current_fiber; }

void ComputeA(SwitchData* data) {
  s32 a{0};
  std::cout << "ComputeA - A0: " << a << '\n';

  Switch(*data->self, *data->next);

  ++a;
  std::cout << "ComputeA - A2: " << a << '\n';

  Switch(*data->self, *data->next);
}

void ComputeB(SwitchData* data) {
  s32 a{1337};
  std::cout << "ComputeB - A1337: " << a << '\n';
  ++a;
  std::cout << "ComputeB - A1338: " << a << '\n';

  Switch(*data->self, *data->next);
}

Fiber* main;
Fiber* fiber_a;
Fiber* fiber_b;

void TestFibers() {
  main = SwitchThreadToFiber();

  SwitchData data_a{&fiber_a, &main};
  SwitchData data_b{&fiber_b, &main};

  [[maybe_unused]] auto* data_a_p{&data_a};
  [[maybe_unused]] auto* data_b_p{&data_b};

  fiber_a = Generate(kNormalStack, ComputeA, &data_a);
  fiber_b = Generate(kNormalStack, ComputeB, &data_b);

  Switch(main, fiber_a);
  Switch(main, fiber_b);

  s32 a{42};
  std::cout << "TestFibers - A42: " << a << '\n';
  ++a;
  std::cout << "TestFibers - A43: " << a << '\n';

  Switch(main, fiber_a);

  ++a;
  std::cout << "TestFibers - A44: " << a << '\n';

  Destroy(fiber_a);
  Destroy(fiber_b);
  Destroy(main);
}
}  // namespace job
}  // namespace comet