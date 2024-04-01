// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_JOB_JOB_H_
#define COMET_COMET_CORE_CONCURRENCY_JOB_JOB_H_

#include <atomic>

#include "comet/core/concurrency/fiber/fiber_primitive.h"
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

enum class JobStackSize { Unknown = 0, Normal, Large };
const schar* GetJobStackSizeLabel(JobStackSize stack_size);

struct JobDescr {
  JobStackSize stack_size{JobStackSize::Unknown};
  fiber::EntryPoint entry_point{};
  fiber::ParamsHandle params_handle{fiber::kInvalidParamsHandle};
  JobPriority priority{JobPriority::Unknown};
  Counter* counter{nullptr};
};

struct IOJobDescr {
  fiber::EntryPoint entry_point{};
  fiber::ParamsHandle params_handle{fiber::kInvalidParamsHandle};
  Counter* counter{nullptr};
};

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
  static inline fiber::FiberPrimitiveDebugId id_counter_{0};
  fiber::FiberPrimitiveDebugId id_{id_counter_++};
  Counter& counter_;
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_JOB_JOB_H_