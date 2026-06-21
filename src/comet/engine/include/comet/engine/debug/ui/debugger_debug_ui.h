// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_ENGINE_DEBUG_UI_DEBUGGER_DEBUG_UI_H_
#define COMET_ENGINE_DEBUG_UI_DEBUGGER_DEBUG_UI_H_

#include "comet/core/essentials.h"

#ifdef COMET_HAS_PROFILER_DEBUG_UI

#include "comet/engine/debug/ui/profiler/cpu_profiler_displayer.h"
#include "comet/runtime/profiler/profiler.h"

#ifdef COMET_HAS_MEMORY_DEBUG_UI
#include "comet/engine/debug/ui/memory/allocation_tracker_displayer.h"
#endif  // COMET_HAS_MEMORY_DEBUG_UI

namespace comet {
namespace debug {
class DebuggerDebugUi {
 public:
  DebuggerDebugUi() = default;
  DebuggerDebugUi(const DebuggerDebugUi&) = delete;
  DebuggerDebugUi(DebuggerDebugUi&&) = delete;
  DebuggerDebugUi& operator=(const DebuggerDebugUi&) = delete;
  DebuggerDebugUi& operator=(DebuggerDebugUi&&) = delete;
  ~DebuggerDebugUi() = default;

  void Draw(const profiler::ProfilerData& profiler_data,
            const CpuProfilerGraph::Controls& controls);

 private:
  void DrawSessionSection(const profiler::ProfilerData& profiler_data) const;
  void DrawEntitySection(const profiler::ProfilerData& profiler_data) const;
  void DrawPhysicsSection(const profiler::ProfilerData& profiler_data) const;
  void DrawRenderingSection(const profiler::ProfilerData& profiler_data) const;
#ifdef COMET_HAS_MEMORY_DEBUG_UI
  void DrawMemorySection(const profiler::ProfilerData& profiler_data) const;
#endif  // COMET_HAS_MEMORY_DEBUG_UI
  void DrawProfilingSection(const profiler::ProfilerData& profiler_data,
                            const CpuProfilerGraph::Controls& controls);

  CpuProfilerDisplayer cpu_profiler_displayer_{};

#ifdef COMET_HAS_MEMORY_DEBUG_UI
  AllocationTrackerDisplayer allocation_tracker_displayer_{};
#endif  // COMET_HAS_MEMORY_DEBUG_UI
};
}  // namespace debug
}  // namespace comet

#endif  // COMET_HAS_PROFILER_DEBUG_UI

#endif  // COMET_ENGINE_DEBUG_UI_DEBUGGER_DEBUG_UI_H_