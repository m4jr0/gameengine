// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_JOB_H_
#define COMET_COMET_CORE_JOB_JOB_H_

#include "comet/core/job/fiber/fiber.h"
#include "comet/core/job/fiber/fiber_primitive.h"

#include "comet/core/type/primitive.h"

#include <mutex>
#include <condition_variable>

namespace comet {
namespace job {
enum class JobPriority { Unknown = 0, Low, Normal, High };

using CounterCount = uindex;
constexpr auto kInvalidCounterCount{static_cast<CounterCount>(-1)};

struct Counter {
  FiberMutex mutex{};
  FiberCV cv{};
  CounterCount value{kInvalidCounterCount};
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
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_JOB_H_