// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_JOB_JOB_UTILS_H_
#define COMET_COMET_CORE_CONCURRENCY_JOB_JOB_UTILS_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"

namespace comet {
namespace job {
JobDescr GenerateJobDescr(
    JobPriority priority, JobEntryPoint entry_point,
    JobParamsHandle params_handle = kInvalidJobParamsHandle,
    JobStackSize stack_size = JobStackSize::Normal, Counter* counter = nullptr,
    const schar* debug_label = nullptr);
IOJobDescr GenerateIOJobDescr(
    IOEntryPoint entry_point,
    IOJobParamsHandle params_handle = kInvalidIOJobParamsHandle,
    Counter* counter = nullptr);
constexpr auto GenerateMainThreadJobDescr = GenerateIOJobDescr;
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_JOB_JOB_UTILS_H_