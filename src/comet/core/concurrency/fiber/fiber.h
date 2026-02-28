// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_H_
#define COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/platform_allocator.h"

namespace comet {
namespace fiber {
using FiberId = usize;
constexpr auto kInvalidFiberId{static_cast<FiberId>(-1)};

using ParamsHandle = void*;
constexpr auto kInvalidParamsHandle{nullptr};

class Fiber;
using OnFiberEndCallback = void (*)(Fiber*, void*);
using EntryPoint = void (*)(ParamsHandle);

// Callees expect the tack to be misaligned by sizeof(uptr) due to the call from
// the caller. Therefore, when generating our own stack, we must simulate that
// offset ourselves.
constexpr auto kStackCallingOffset{sizeof(uptr)};

using RegisterValue = ux;
constexpr auto kInvalidRegisterValue{0};

#if defined(COMET_MSVC) && defined(COMET_ARCH_X86)
using Sse2RegisterValue = ms128;
const auto kInvalidSse2RegisterValue{_mm_set_epi32(0, 0, 0, 0)};
#endif  // defined(COMET_MSVC) && defined(COMET_ARCH_X86)

struct ExecutionContext {
  RegisterValue rbx{kInvalidRegisterValue};
  RegisterValue rbp{kInvalidRegisterValue};
#ifdef COMET_MSVC
  RegisterValue rdi{kInvalidRegisterValue};
  RegisterValue rsi{kInvalidRegisterValue};
#endif  // COMET_MSVC
  RegisterValue r12{kInvalidRegisterValue};
  RegisterValue r13{kInvalidRegisterValue};
  RegisterValue r14{kInvalidRegisterValue};
  RegisterValue r15{kInvalidRegisterValue};
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
  // data argument.
#ifdef COMET_MSVC
  RegisterValue rcx{kInvalidRegisterValue};
#else
  RegisterValue rdi{kInvalidRegisterValue};
#endif  // COMET_MSVC
};

struct SwitchData;

using FiberFunc = void (*)(SwitchData*);

class Fiber {
 public:
  using Stack = u8*;
  static usize GetAllocatedStackSize(usize capacity);

  Fiber();  // Create empty fiber, usually from a thread.
  explicit Fiber(usize stack_size);
  Fiber(const Fiber&) = delete;
  Fiber(Fiber&&) = delete;
  Fiber& operator=(const Fiber&) = delete;
  Fiber& operator=(Fiber&&) = delete;
  ~Fiber() = default;

  void Initialize();
  void Destroy();
  void Reset();

  void Attach(EntryPoint entry_point, ParamsHandle params_handle,
              OnFiberEndCallback end_callback = nullptr,
              void* end_callback_data = nullptr,
              const schar* debug_label = nullptr);
  void Detach();
  static void Run(Fiber* fiber);

  ExecutionContext& GetContext() noexcept;
  FiberId GetId() const noexcept;
  sptrdiff GetCurrentStackSize() const;
  const void* GetStack() const noexcept;
  usize GetStackCapacity() const noexcept;
  sptrdiff GetCurrentStackSizeLeft() const;
  bool IsStackOverflow() const;

#ifdef COMET_FIBER_DEBUG_LABEL
  const schar* GetDebugLabel() const noexcept;

  static inline constexpr usize kDebugLabelMaxLen_{31};
  static inline constexpr schar kDefaultDebugLabel_[4]{"???"};
#endif  // COMET_FIBER_DEBUG_LABEL

 private:
  inline static constexpr usize kThreadStacktraceSize_{kUSizeMax};

  static_assert(
      std::atomic<FiberId>::is_always_lock_free,
      "std::atomic<FiberId> needs to be always lock-free. Unsupported "
      "architecture");
  static inline std::atomic<FiberId> id_counter_{0};

#ifdef COMET_POISON_FIBER_STACKS
  void PoisonStack();
#endif  // COMET_POISON_FIBER_STACKS

  FiberId id_{id_counter_.fetch_add(1)};

#ifdef COMET_FIBER_DEBUG_LABEL
  schar debug_label_[kDebugLabelMaxLen_ + 1]{"never_used\0"};
#endif  // COMET_FIBER_DEBUG_LABEL

  EntryPoint entry_point_{nullptr};
  ParamsHandle params_handle_{kInvalidParamsHandle};
  OnFiberEndCallback end_callback_{nullptr};
  void* end_callback_data_{nullptr};
  usize stack_capacity_{0};
  Stack stack_{nullptr};
  uptr* stack_top_{nullptr};
  ExecutionContext context_{};
};

constexpr auto kMicroStackSize{2048};       // 2 KiB.
constexpr auto kTinyStackSize{4096};        // 4 KiB.
constexpr auto kSmallStackSize{8192};       // 8 KiB.
constexpr auto kNormalStackSize{16384};     // 16 KiB.
constexpr auto kElevatedStackSize{32768};   // 32 KiB.
constexpr auto kLargeStackSize{65536};      // 64 KiB.
constexpr auto kHugeStackSize{131072};      // 128 KiB.
constexpr auto kGiantStackSize{262144};     // 256 KiB.
constexpr auto kGiganticStackSize{524288};  // 512 KiB.

#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
constexpr auto kSmallExternalLibraryStackSize{2097152};   // 2 MiB.
constexpr auto kNormalExternalLibraryStackSize{4194304};  // 4 MiB.
constexpr auto kBigExternalLibraryStackSize{8388608};     // 8 MiB.
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

namespace internal {
class FiberInternalAllocator {
 public:
  static FiberInternalAllocator& Get();

  FiberInternalAllocator() = default;
  explicit FiberInternalAllocator(usize capacity);
  FiberInternalAllocator(const FiberInternalAllocator&) = delete;
  FiberInternalAllocator(FiberInternalAllocator&& other) noexcept;
  FiberInternalAllocator& operator=(const FiberInternalAllocator&) = delete;
  FiberInternalAllocator& operator=(FiberInternalAllocator&& other) noexcept;
  ~FiberInternalAllocator();

  void Initialize();
  void Destroy();

  Fiber::Stack Allocate(usize size);
  void Deallocate(Fiber::Stack);

  bool IsInitialized() const noexcept;

 private:
  bool is_initialized_{false};
  memory::PlatformStackAllocator allocator_{};
};
}  // namespace internal

void AllocateFiberStackMemory(usize capacity);
void DestroyFiberStackMemory();
}  // namespace fiber
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_H_