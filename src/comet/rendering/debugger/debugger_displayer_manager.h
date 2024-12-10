// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DEBUGGER_DEBUGGUER_DISPLAYER_MANAGER_H_
#define COMET_COMET_RENDERING_DEBUGGER_DEBUGGUER_DISPLAYER_MANAGER_H_

#include "comet/core/essentials.h"

#ifdef COMET_PROFILING
#include "comet/core/manager.h"
#include "comet/core/memory/memory.h"
#include "comet/profiler/profiler_data.h"

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
#ifdef COMET_IMGUI
  void DrawPhysicsSection(const profiler::ProfilerData& profiler_data) const;
  void DrawRenderingSection(const profiler::ProfilerData& profiler_data) const;
  void DrawMemorySection(const profiler::ProfilerData& profiler_data) const;
#endif  // COMET_IMGUI
};
}  // namespace rendering
}  // namespace comet
#endif  // COMET_PROFILING

#endif  // COMET_COMET_RENDERING_DEBUGGER_DEBUGGUER_DISPLAYER_MANAGER_H_
