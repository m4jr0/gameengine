// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/job/job_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace job {
const schar* GetJobPriorityLabel(JobPriority priority) {
  switch (priority) {
    case JobPriority::Unknown:
      return "unknown";
    case JobPriority::Low:
      return "low";
    case JobPriority::Normal:
      return "normal";
    case JobPriority::High:
      return "high";
    default:
      return kUnknownLabel;
  }
}

const schar* GetJobStackSizeLabel(JobStackSize stack_size) {
  switch (stack_size) {
    case JobStackSize::Unknown:
      return "unknown";
    case JobStackSize::Normal:
      return "normal";
    case JobStackSize::Large:
      return "large";
#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
    case JobStackSize::ExternalLibrary:
      return "external_library";
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  }

  return kUnknownLabel;
}
}  // namespace job
}  // namespace comet