// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_JOB_JOB_LABEL_H_
#define COMET_CORE_JOB_JOB_LABEL_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"

namespace comet {
namespace job {
const schar* GetJobPriorityLabel(JobPriority priority);
const schar* GetJobStackSizeLabel(JobStackSize stack_size);
}  // namespace job
}  // namespace comet

#endif  // COMET_CORE_JOB_JOB_LABEL_H_