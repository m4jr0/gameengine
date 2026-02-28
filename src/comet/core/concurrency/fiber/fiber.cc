// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "fiber.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_context.h"
#include "comet/core/logger.h"
#include "comet/core/memory/memory.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"

#ifdef COMET_FIBER_DEBUG_LABEL
#include "comet/core/c_string.h"
#include "comet/math/math_common.h"
#endif  // COMET_FIBER_DEBUG_LABEL

// TODO(m4jr0): Support other architectures.
#ifndef COMET_ARCH_X86_64
static_assert(false, "Unsupported architecture.");
#endif  // !COMET_ARCH_X86_64

namespace comet {
namespace fiber {
usize Fiber::GetAllocatedStackSize(usize capacity) {
#ifdef COMET_MSVC
  constexpr auto kRedZoneOffset{0};
#else
  constexpr auto kRedZoneOffset{128};
#endif  // COMET_MSVC

  return capacity + kRedZoneOffset;
}

void Fiber::Initialize() {
  COMET_ASSERT(stack_capacity_ != kThreadStacktraceSize_,
               "Cannot initialize a thread fiber!");
  stack_ = internal::FiberInternalAllocator::Get().Allocate(
      GetAllocatedStackSize(stack_capacity_));

#ifdef COMET_POISON_FIBER_STACKS
  PoisonStack();
#endif  // COMET_POISON_FIBER_STACKS

  // On supported architectures, stack grows downwards.
  stack_top_ = reinterpret_cast<uptr*>(stack_ + stack_capacity_);

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
    internal::FiberInternalAllocator::Get().Deallocate(stack_);
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
                   OnFiberEndCallback end_callback, void* end_callback_data,
                   [[maybe_unused]] const schar* debug_label) {
  entry_point_ = entry_point;
  params_handle_ = params_handle;
  end_callback_ = end_callback;
  end_callback_data_ = end_callback_data;

#ifdef COMET_FIBER_DEBUG_LABEL
  if (debug_label == nullptr) {
    debug_label = kDefaultDebugLabel_;
  }

  auto len{math::Min(GetLength(debug_label), kDebugLabelMaxLen_)};
  Copy(debug_label_, debug_label, len);
  debug_label_[len + 1] = '\0';
#endif  // COMET_FIBER_DEBUG_LABEL
}

void Fiber::Detach() {
  entry_point_ = nullptr;
  params_handle_ = kInvalidParamsHandle;
  end_callback_ = nullptr;
  end_callback_data_ = nullptr;
#ifdef COMET_FIBER_DEBUG_LABEL
  Copy(debug_label_, "detached\0", 9);
#endif  // COMET_FIBER_DEBUG_LABEL
}

void Fiber::Run(Fiber* fiber) {
  COMET_ASSERT(fiber->entry_point_ != nullptr,
               "Entry point is null! Did you attach it?");
  fiber->entry_point_(fiber->params_handle_);

  if (fiber->end_callback_ != nullptr) {
    fiber->end_callback_(fiber, fiber->end_callback_data_);
  }
}

ExecutionContext& Fiber::GetContext() noexcept { return context_; }

FiberId Fiber::GetId() const noexcept { return id_; }

sptrdiff Fiber::GetCurrentStackSize() const {
  COMET_ASSERT(GetFiber() == this,
               "Cannot compute current stack size from another fiber!");
  // No stack overflow checks for fibers; the OS handles the large stack.
  if (stack_capacity_ == kThreadStacktraceSize_) {
    return stack_capacity_;
  }

  u8 anchor{};
  return &anchor - stack_;
}

const void* Fiber::GetStack() const noexcept { return stack_; }

usize Fiber::GetStackCapacity() const noexcept { return stack_capacity_; }

sptrdiff Fiber::GetCurrentStackSizeLeft() const {
  auto current_stack_size{GetCurrentStackSize()};
  sptrdiff stack_capacity{static_cast<sptrdiff>(stack_capacity_)};

  return stack_capacity - current_stack_size;
}

bool Fiber::IsStackOverflow() const {
  auto size_left{GetCurrentStackSizeLeft()};

  if (size_left < 0) {
    COMET_LOG_CORE_ERROR("Stack overflow of ", -size_left,
                         " bytes has been detected!");
  }

  return size_left < 0;
}

#ifdef COMET_FIBER_DEBUG_LABEL
const schar* Fiber::GetDebugLabel() const noexcept { return debug_label_; }
#endif  // COMET_FIBER_DEBUG_LABEL

#ifdef COMET_POISON_FIBER_STACKS
void Fiber::PoisonStack() {
  COMET_ASSERT(stack_ != nullptr, "Stack is null!");
  COMET_ASSERT(stack_capacity_ != 0, "Stack capacity is 0!");
  constexpr StaticArray<u8, 8> kPoison{0xde, 0xad, 0xbe, 0xef,
                                       0xde, 0xad, 0xbe, 0xef};
  constexpr auto kPoisonLen{kPoison.GetSize()};
  constexpr auto kAllocationIdSize{sizeof(id_)};
  COMET_ASSERT(kPoisonLen == kAllocationIdSize,
               "Poison and allocation ID size misalign. This would cause "
               "poison drifting when investigating.");

  usize i{0};
  auto* cur{stack_};
  auto* top{cur + stack_capacity_};

  // Fill misaligned bytes with poison first.
  auto* aligned_cur{reinterpret_cast<u8*>(
      memory::AlignAddress(reinterpret_cast<uptr>(cur), kAllocationIdSize))};

  if (aligned_cur > top) {
    aligned_cur = top;
  }

  while (cur < aligned_cur) {
    *cur = kPoison[i % kPoisonLen];
    ++cur;
    ++i;
  }

  auto is_id{false};

  while (cur + kAllocationIdSize <= top) {
    if (is_id) {
      memory::CopyMemory(cur, &id_, kAllocationIdSize);
    } else {
      memory::CopyMemory(cur, kPoison.GetData(), kPoisonLen);
    }

    is_id = !is_id;
    cur += kAllocationIdSize;
  }

  while (cur < top) {
    *cur = kPoison[i % kPoisonLen];
    ++i;
    ++cur;
  }
}
#endif  // COMET_POISON_FIBER_STACKS

Fiber::Fiber() : stack_capacity_{kThreadStacktraceSize_} {
#ifdef COMET_FIBER_DEBUG_LABEL
  Copy(debug_label_, "thread\0", 7);
#endif  // COMET_FIBER_DEBUG_LABEL
}

Fiber::Fiber(usize stack_size) : stack_capacity_{stack_size} {
  COMET_ASSERT(stack_capacity_ != 0, "Stack size provided is 0!");
}

namespace internal {
FiberInternalAllocator::~FiberInternalAllocator() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for fiber internal allocator, but it is "
               "still initialized!");
}

FiberInternalAllocator& FiberInternalAllocator::Get() {
  static FiberInternalAllocator singleton{};
  return singleton;
}

FiberInternalAllocator::FiberInternalAllocator(usize capacity)
    : allocator_{capacity, memory::kEngineMemoryTagFiber} {}

FiberInternalAllocator::FiberInternalAllocator(
    FiberInternalAllocator&& other) noexcept
    : is_initialized_{other.is_initialized_},
      allocator_{std::move(other.allocator_)} {
  other.is_initialized_ = false;
}

FiberInternalAllocator& FiberInternalAllocator::operator=(
    FiberInternalAllocator&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  is_initialized_ = other.is_initialized_;
  allocator_ = std::move(other.allocator_);

  other.is_initialized_ = false;
  return *this;
}

void FiberInternalAllocator::Initialize() {
  COMET_ASSERT(
      !is_initialized_,
      "Tried to initialize fiber internal allocator, but it is already done!");
  allocator_.Initialize();
  is_initialized_ = true;
}

void FiberInternalAllocator::Destroy() {
  COMET_ASSERT(
      is_initialized_,
      "Tried to destroy fiber internal allocator, but it is not initialized!");
  allocator_.Destroy();
  is_initialized_ = false;
}

Fiber::Stack FiberInternalAllocator::Allocate(usize size) {
  return static_cast<Fiber::Stack>(allocator_.Allocate(size));
}

void FiberInternalAllocator::Deallocate(Fiber::Stack) {
  // The fiber's internal allocator assumes that all fibers are created and
  // destroyed simultaneously. This is why it utilizes a stack allocator
  // internally. A Deallocate method is still provided in case this strategy
  // changes in the future, and fibers continue to use it to maintain
  // consistency.
}

bool FiberInternalAllocator::IsInitialized() const noexcept {
  return is_initialized_;
}
}  // namespace internal

void AllocateFiberStackMemory(usize capacity) {
  auto& allocator{internal::FiberInternalAllocator::Get()};
  allocator = internal::FiberInternalAllocator{capacity};
  allocator.Initialize();
}

void DestroyFiberStackMemory() {
  internal::FiberInternalAllocator::Get().Destroy();
}
// namespace internal
}  // namespace fiber
}  // namespace comet