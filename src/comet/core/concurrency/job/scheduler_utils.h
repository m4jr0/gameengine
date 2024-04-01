// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_SCHEDULER_UTILS_H_
#define COMET_COMET_CORE_CONCURRENCY_SCHEDULER_UTILS_H_

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/essentials.h"

namespace comet {
namespace job {
JobDescr GenerateJobDescr(
    JobPriority priority, fiber::EntryPoint entry_point,
    fiber::ParamsHandle params_handle = fiber::kInvalidParamsHandle,
    JobStackSize stack_size = JobStackSize::Normal, Counter* counter = nullptr);
IOJobDescr GenerateIOJobDescr(
    fiber::EntryPoint entry_point,
    fiber::ParamsHandle params_handle = fiber::kInvalidParamsHandle,
    Counter* counter = nullptr);
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_SCHEDULER_UTILS_H_