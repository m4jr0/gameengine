// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DEBUGGER_DEBUGGUER_DISPLAYER_MANAGER_H_
#define COMET_COMET_RENDERING_DEBUGGER_DEBUGGUER_DISPLAYER_MANAGER_H_

#include "comet/core/essentials.h"

#ifdef COMET_DEBUG
#include "comet/core/manager.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/debugger/memory/allocation_tracker_displayer.h"
#include "comet/rendering/debugger/profiler/cpu_profiler_displayer.h"

namespace comet {
namespace rendering {
class DebuggerDisplayerManager : public Manager {
 public:
  static DebuggerDisplayerManager& Get();

  DebuggerDisplayerManager() = default;
  DebuggerDisplayerManager(const DebuggerDisplayerManager&) = delete;
  DebuggerDisplayerManager(DebuggerDisplayerManager&&) = delete;
  DebuggerDisplayerManager& operator=(const DebuggerDisplayerManager&) = delete;
  DebuggerDisplayerManager& operator=(DebuggerDisplayerManager&&) = delete;
  virtual ~DebuggerDisplayerManager() = default;

  void Draw();

 private:
#ifdef COMET_PROFILING
#ifdef COMET_IMGUI
  void DrawPhysicsSection(const profiler::ProfilerData& profiler_data) const;
  void DrawRenderingSection(const profiler::ProfilerData& profiler_data) const;
  void DrawMemorySection(const profiler::ProfilerData& profiler_data) const;
  void DrawProfilingSection(const profiler::ProfilerData& profiler_data);

  CpuProfilerDisplayer cpu_profiler_displayer_{};
  AllocationTrackerDisplayer allocation_tracker_displayer_{};
#endif  // COMET_IMGUI
#endif  // COMET_PROFILING
};
}  // namespace rendering
}  // namespace comet
#endif  // COMET_DEBUG

#endif  // COMET_COMET_RENDERING_DEBUGGER_DEBUGGUER_DISPLAYER_MANAGER_H_
