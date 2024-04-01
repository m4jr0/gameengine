// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_H_
#define COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_H_

#include <atomic>

#include "comet/core/essentials.h"

namespace comet {
namespace fiber {
using FiberId = usize;
constexpr auto kInvalidFiberId{static_cast<FiberId>(-1)};

using ParamsHandle = uptr;
constexpr auto kInvalidParamsHandle{0};

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

enum class FiberState { Unknown = 0, Pending, Running, Suspended };

using FiberFunc = void (*)(SwitchData*);

class Fiber {
 public:
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
              void* end_callback_data = nullptr);
  void Detach();
  static void Run(Fiber* fiber);
  bool IsRunning() const noexcept;
  bool IsYielded() const noexcept;

  ExecutionContext& GetContext() noexcept;
  FiberId GetId() const noexcept;
  FiberState GetState() const noexcept;

 private:
  static_assert(
      std::atomic<FiberId>::is_always_lock_free,
      "std::atomic<FiberId> needs to be always lock-free. Unsupported "
      "architecture");
  static inline std::atomic<FiberId> id_counter_{0};

  FiberId id_{id_counter_.fetch_add(1)};
  FiberState state_{FiberState::Pending};
  EntryPoint entry_point_{nullptr};
  ParamsHandle params_handle_{kInvalidParamsHandle};
  OnFiberEndCallback end_callback_{nullptr};
  void* end_callback_data_{nullptr};
  usize stack_size_{0};
  u8* stack_{nullptr};
  uptr* stack_top_{nullptr};
  ExecutionContext context_{};
};

constexpr auto kMicroStack{2048};       // 2 KiB.
constexpr auto kTinyStack{4096};        // 4 KiB.
constexpr auto kSmallStack{8192};       // 8 KiB.
constexpr auto kNormalStack{16384};     // 16 KiB.
constexpr auto kElevatedStack{32768};   // 32 KiB.
constexpr auto kLargeStack{65536};      // 64 KiB.
constexpr auto kHugeStack{131072};      // 128 KiB.
constexpr auto kGiantStack{262144};     // 256 KiB.
constexpr auto kGiganticStack{524288};  // 512 KiB.
}  // namespace fiber
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_FIBER_FIBER_H_