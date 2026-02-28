// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DEBUGGER_MEMORY_ALLOCATION_TRACKER_DISPLAYER_H_
#define COMET_COMET_RENDERING_DEBUGGER_MEMORY_ALLOCATION_TRACKER_DISPLAYER_H_

#include "comet/core/essentials.h"

#ifdef COMET_PROFILING
#ifdef COMET_IMGUI
#include "comet/profiler/profiler.h"
#include "comet/rendering/debugger/profiler/cpu_profiler_displayer.h"

namespace comet {
namespace rendering {
class AllocationTrackerDisplayer {
 public:
  AllocationTrackerDisplayer() = default;
  AllocationTrackerDisplayer(const AllocationTrackerDisplayer&) = delete;
  AllocationTrackerDisplayer(AllocationTrackerDisplayer&&) = delete;
  AllocationTrackerDisplayer& operator=(const AllocationTrackerDisplayer&) =
      delete;
  AllocationTrackerDisplayer& operator=(AllocationTrackerDisplayer&&) = delete;
  virtual ~AllocationTrackerDisplayer() = default;

  void Draw(const profiler::ProfilerData& profiler_data) const;
};
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI
#endif  // COMET_PROFILING

#endif  // COMET_COMET_RENDERING_DEBUGGER_MEMORY_ALLOCATION_TRACKER_DISPLAYER_H_
