// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_SCHEDULER_UTILS_H_
#define COMET_COMET_CORE_SCHEDULER_UTILS_H_

#include "comet/core/job/job.h"
#include "comet/core/job/scheduler.h"

namespace comet {
namespace job {
JobDescr GenerateJobDescr(JobPriority priority, EntryPoint entry_point,
                          ParamsHandle params_handle = kInvalidParamsHandle,
                          Counter* counter = nullptr);
IOJobDescr GenerateIOJobDescr(EntryPoint entry_point,
                              ParamsHandle params_handle = kInvalidParamsHandle,
                              Counter* counter = nullptr);
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_SCHEDULER_UTILS_H_