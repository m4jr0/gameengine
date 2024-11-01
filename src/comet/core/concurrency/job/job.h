// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_JOB_JOB_H_
#define COMET_COMET_CORE_CONCURRENCY_JOB_JOB_H_

#include <atomic>

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/essentials.h"

namespace comet {
namespace job {
enum class JobPriority { Unknown = 0, Low, Normal, High };

using CounterCount = usize;
constexpr auto kInvalidCounterCount{static_cast<CounterCount>(-1)};

class Counter {
 public:
  Counter() = default;
  Counter(const Counter&) = default;
  Counter(Counter&&) = default;
  Counter& operator=(const Counter&) = default;
  Counter& operator=(Counter&&) = default;
  ~Counter() = default;

  void Reset();
  void Increment();
  void Decrement();
  bool IsZero() const noexcept;

  usize GetValue() const noexcept;

 private:
  std::atomic<CounterCount> value_{kInvalidCounterCount};
};

enum class JobStackSize {
  Unknown = 0,
  Normal,
  Large,
#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  ExternalLibrary
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
};
const schar* GetJobStackSizeLabel(JobStackSize stack_size);

using JobEntryPoint = fiber::EntryPoint;
using JobParamsHandle = fiber::ParamsHandle;
constexpr auto kInvalidJobParamsHandle{nullptr};

struct JobDescr {
  JobStackSize stack_size{JobStackSize::Unknown};
  JobEntryPoint entry_point{};
  JobParamsHandle params_handle{kInvalidJobParamsHandle};
  JobPriority priority{JobPriority::Unknown};
  Counter* counter{nullptr};
#ifdef COMET_FIBER_DEBUG_LABEL
  schar debug_label[fiber::Fiber::kDebugLabelMaxLen_]{"no_label"};
#endif  // COMET_FIBER_DEBUG_LABEL
};

using IOJobParamsHandle = void*;
constexpr auto kInvalidIOJobParamsHandle{nullptr};

using IOEntryPoint = void (*)(IOJobParamsHandle);

struct IOJobDescr {
  IOEntryPoint entry_point{};
  IOJobParamsHandle params_handle{kInvalidIOJobParamsHandle};
  Counter* counter{nullptr};
};

#ifdef COMET_DEBUG
using JobPrimitiveDebugId = usize;
constexpr auto kInvalidJobPrimitiveDebugId{
    static_cast<JobPrimitiveDebugId>(-1)};
#endif  // COMET_DEBUG

class CounterGuard {
 public:
  CounterGuard() = delete;
  explicit CounterGuard(Counter& counter);
  CounterGuard(const CounterGuard&) = delete;
  CounterGuard(CounterGuard&&) = delete;
  CounterGuard& operator=(const CounterGuard&) = delete;
  CounterGuard& operator=(CounterGuard&&) = delete;
  ~CounterGuard() = default;

 private:
#ifdef COMET_DEBUG
  static_assert(std::atomic<JobPrimitiveDebugId>::is_always_lock_free,
                "std::atomic<JobPrimitiveDebugId> needs to be always "
                "lock-free. Unsupported "
                "architecture");
  static inline std::atomic<JobPrimitiveDebugId> id_counter_{0};
  JobPrimitiveDebugId id_{id_counter_++};
#endif  // COMET_DEBUG
  Counter& counter_;
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_JOB_JOB_H_