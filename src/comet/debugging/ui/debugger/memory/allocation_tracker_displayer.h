// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_DEBUGGING_UI_DEBUGGER_MEMORY_ALLOCATION_TRACKER_DISPLAYER_H_
#define COMET_COMET_DEBUGGING_UI_DEBUGGER_MEMORY_ALLOCATION_TRACKER_DISPLAYER_H_

#include "comet/core/essentials.h"

#ifdef COMET_HAS_MEMORY_DEBUG_UI

#include "comet/profiler/profiler.h"

namespace comet {
namespace debug {
class AllocationTrackerDisplayer {
 public:
  AllocationTrackerDisplayer() = default;
  AllocationTrackerDisplayer(const AllocationTrackerDisplayer&) = delete;
  AllocationTrackerDisplayer(AllocationTrackerDisplayer&&) = delete;
  AllocationTrackerDisplayer& operator=(const AllocationTrackerDisplayer&) =
      delete;
  AllocationTrackerDisplayer& operator=(AllocationTrackerDisplayer&&) = delete;
  ~AllocationTrackerDisplayer() = default;

  void Draw(const profiler::ProfilerData& profiler_data) const;
};
}  // namespace debug
}  // namespace comet

#endif  // COMET_HAS_MEMORY_DEBUG_UI

#endif  // COMET_COMET_DEBUGGING_UI_DEBUGGER_MEMORY_ALLOCATION_TRACKER_DISPLAYER_H_