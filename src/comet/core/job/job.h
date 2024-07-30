// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_JOB_H_
#define COMET_COMET_CORE_JOB_JOB_H_

#include <condition_variable>
#include <mutex>

#include "comet/core/job/fiber/fiber.h"
#include "comet/core/job/fiber/fiber_primitive.h"
#include "comet/core/type/primitive.h"

namespace comet {
namespace job {
enum class JobPriority { Unknown = 0, Low, Normal, High };

using CounterCount = uindex;
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

  uindex GetValue() const noexcept;

 private:
  std::atomic<CounterCount> value_{kInvalidCounterCount};
};

struct JobDescr {
  EntryPoint entry_point{};
  ParamsHandle params_handle{kInvalidParamsHandle};
  JobPriority priority{JobPriority::Unknown};
  Counter* counter{nullptr};
};

struct IOJobDescr {
  EntryPoint entry_point{};
  ParamsHandle params_handle{kInvalidParamsHandle};
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
  static inline FiberPrimitiveId id_counter_{0};
  FiberPrimitiveId id_{id_counter_++};
  Counter& counter_;
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_JOB_H_