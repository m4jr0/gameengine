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
namespace fiber {
void Fiber::Initialize() {
#ifdef COMET_MSVC
  constexpr auto kRedZoneOffset{0};
#else
  constexpr auto kRedZoneOffset{128};
#endif  // COMET_MSVC

  stack_ = static_cast<u8*>(
      memory::Allocate(stack_size_ + kRedZoneOffset, memory::MemoryTag::Fiber));
  // On supported architectures, stack grows downwards.
  stack_top_ = reinterpret_cast<uptr*>(stack_ + stack_size_);

  // Align stack before setting RSP.
  stack_top_ = reinterpret_cast<uptr*>(
      reinterpret_cast<u8*>((reinterpret_cast<uptr>(&stack_top_[-1]) &
                             -memory::kStackAlignment)) -
      kStackCallingOffset);
  *stack_top_ = 0x0;  // Set return address to 0.

  Reset();
}

void Fiber::Destroy() {
  if (stack_ != nullptr) {
    memory::Deallocate(stack_);
    stack_ = nullptr;
  }
}

void Fiber::Reset() {
  context_.rsp = reinterpret_cast<RegisterValue>(stack_top_);

  context_.rip = reinterpret_cast<RegisterValue>(Fiber::Run);

#ifdef COMET_MSVC
  context_.rcx = reinterpret_cast<RegisterValue>(this);
#else
  context_.rdi = reinterpret_cast<RegisterValue>(this);
#endif  // COMET_MSVC
}

void Fiber::Attach(EntryPoint entry_point, ParamsHandle params_handle,
                   OnFiberEndCallback end_callback, void* end_callback_data) {
  entry_point_ = entry_point;
  params_handle_ = params_handle;
  end_callback_ = end_callback;
  end_callback_data_ = end_callback_data;
}

void Fiber::Detach() {
  entry_point_ = nullptr;
  params_handle_ = kInvalidParamsHandle;
  end_callback_ = nullptr;
  end_callback_data_ = nullptr;
}

void Fiber::Run(Fiber* fiber) {
  COMET_ASSERT(fiber->entry_point_ != nullptr,
               "Entry point is null! Did you attach it?");
  fiber->entry_point_(fiber->params_handle_);

  if (fiber->end_callback_ != nullptr) {
    fiber->end_callback_(fiber, fiber->end_callback_data_);
  }
}

bool Fiber::IsRunning() const noexcept { return state_ == FiberState::Running; }

bool Fiber::IsYielded() const noexcept {
  return state_ == FiberState::Suspended;
}

ExecutionContext& Fiber::GetContext() noexcept { return context_; }

FiberId Fiber::GetId() const noexcept { return id_; }

FiberState Fiber::GetState() const noexcept { return state_; }

Fiber::Fiber() : state_{FiberState::Running} {}

Fiber::Fiber(usize stack_size) : stack_size_{stack_size} {
  COMET_ASSERT(stack_size_ != 0, "Stack size provided is 0!");
}
}  // namespace fiber
}  // namespace comet