// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "job_utils.h"

#ifdef COMET_FIBER_DEBUG_LABEL
#include "comet/core/c_string.h"
#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/math/math_commons.h"
#endif  // COMET_FIBER_DEBUG_LABEL

namespace comet {
namespace job {
JobDescr GenerateJobDescr(JobPriority priority, JobEntryPoint entry_point,
                          JobParamsHandle params_handle,
                          JobStackSize stack_size, Counter* counter,
                          [[maybe_unused]] const schar* debug_label) {
  JobDescr descr{};
  descr.stack_size = stack_size;
  descr.priority = priority;
  descr.entry_point = entry_point;
  descr.params_handle = params_handle;
  descr.counter = counter;
#ifdef COMET_FIBER_DEBUG_LABEL
  if (debug_label == nullptr) {
    debug_label = fiber::Fiber::kDefaultDebugLabel_;
  }

  auto len{math::Min(GetLength(debug_label), fiber::Fiber::kDebugLabelMaxLen_)};
  Copy(descr.debug_label, debug_label, len);
  descr.debug_label[len + 1] = '\0';
#endif  // COMET_FIBER_DEBUG_LABEL
  return descr;
}

IOJobDescr GenerateIOJobDescr(IOEntryPoint entry_point,
                              IOJobParamsHandle params_handle,
                              Counter* counter) {
  IOJobDescr descr{};
  descr.entry_point = entry_point;
  descr.params_handle = params_handle;
  descr.counter = counter;
  return descr;
}
}  // namespace job
}  // namespace comet