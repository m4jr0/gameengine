// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_FIBER_H_
#define COMET_COMET_CORE_TYPE_FIBER_H_

#include "comet_precompile.h"

namespace comet {
namespace job {
using RegisterValue = ux;
constexpr auto kInvalidRegisterValue{0};

#if defined(COMET_MSVC) && defined(COMET_ARCH_X86)
using Sse2RegisterValue = ms128;
const auto kInvalidSse2RegisterValue{_mm_set_epi32(0, 0, 0, 0)};
#endif  // defined(COMET_MSVC) && defined(COMET_ARCH_X86)

struct ExecutionContext {
  RegisterValue rbx{kInvalidRegisterValue};
  RegisterValue rbp{kInvalidRegisterValue};
  RegisterValue rdi{kInvalidRegisterValue};
  RegisterValue rsi{kInvalidRegisterValue};
  RegisterValue r12{kInvalidRegisterValue};
  RegisterValue r13{kInvalidRegisterValue};
  RegisterValue r14{kInvalidRegisterValue};
  RegisterValue r15{kInvalidRegisterValue};
  // data argument.
#ifdef COMET_MSVC
  RegisterValue rcx{kInvalidRegisterValue};
#else
  RegisterValue rdi{kInvalidRegisterValue};
#endif  // COMET_MSVC
  RegisterValue rsp{kInvalidRegisterValue};
  RegisterValue rip{kInvalidRegisterValue};
#if defined(COMET_MSVC) && defined(COMET_ARCH_X86)
  Sse2RegisterValue xmm6{kInvalidSse2RegisterValue};
  Sse2RegisterValue xmm7{kInvalidSse2RegisterValue};
#ifdef COMET_ARCH_X86_64
  Sse2RegisterValue xmm8{kInvalidSse2RegisterValue};
  Sse2RegisterValue xmm9{kInvalidSse2RegisterValue};
  Sse2RegisterValue xmm10{kInvalidSse2RegisterValue};
  Sse2RegisterValue xmm11{kInvalidSse2RegisterValue};
  Sse2RegisterValue xmm12{kInvalidSse2RegisterValue};
  Sse2RegisterValue xmm13{kInvalidSse2RegisterValue};
  Sse2RegisterValue xmm14{kInvalidSse2RegisterValue};
  Sse2RegisterValue xmm15{kInvalidSse2RegisterValue};
#endif  // COMET_ARCH_X86_64
#endif  // defined(COMET_MSVC) && defined(COMET_ARCH_X86)
};

struct Fiber {
  uindex stack_size{0};
  u8* stack{nullptr};
  void* func{nullptr};
  ExecutionContext context{};
};

Fiber* Generate(uindex stack_size, void* func, void* data = nullptr);
void Destroy(Fiber* fiber);
void Switch(Fiber* from, Fiber* to);
Fiber* SwitchThreadToFiber();
void TestFibers();
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_FIBER_H_