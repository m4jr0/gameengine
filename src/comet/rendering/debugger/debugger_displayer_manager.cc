// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "debugger_displayer_manager.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_DEBUG
// External. ///////////////////////////////////////////////////////////////////
#include <algorithm>
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_PROFILING
#include "comet/profiler/profiler_manager.h"
#endif  // COMET_PROFILING

#ifdef COMET_IMGUI
#include "comet/rendering/debugger/imgui_utils.h"
#include "imgui.h"
#endif  // COMET_IMGUI

namespace comet {
namespace rendering {
DebuggerDisplayerManager& DebuggerDisplayerManager::Get() {
  static DebuggerDisplayerManager singleton{};
  return singleton;
}

void DebuggerDisplayerManager::Draw() {
#ifdef COMET_PROFILING
#ifdef COMET_IMGUI
  auto& profiler_data{profiler::ProfilerManager::Get().GetData()};

  ImGui::SetNextWindowSizeConstraints(ImVec2{450, 300},
                                      // No limit for max size.
                                      ImVec2{FLT_MAX, FLT_MAX});

  ImGui::Begin("Mini Profiler");

  DrawPhysicsSection(profiler_data);
  ImGui::Spacing();
  DrawRenderingSection(profiler_data);

#ifdef COMET_TRACK_ALLOCATIONS
  ImGui::Spacing();
  DrawMemorySection(profiler_data);
#endif  // COMET_TRACK_ALLOCATIONS

  ImGui::Spacing();
  DrawProfilingSection(profiler_data);

  ImGui::End();
#endif  // COMET_IMGUI
#endif  // COMET_PROFILING
}

#ifdef COMET_PROFILING
#ifdef COMET_IMGUI
void DebuggerDisplayerManager::DrawPhysicsSection(
    const profiler::ProfilerData& profiler_data) const {
  ImGui::Text("PHYSICS");
  ImGui::Indent();
  ImGui::Text("Frame Time: %f ms", profiler_data.physics_frame_time * 1000);
  ImGui::Text("Framerate: %u Hz", profiler_data.physics_frame_rate);
  ImGui::Unindent();
}

void DebuggerDisplayerManager::DrawRenderingSection(
    const profiler::ProfilerData& profiler_data) const {
  ImGui::Text("RENDERING");
  ImGui::Indent();
  ImGui::Text("Driver: %s",
              GetDriverTypeLabel(profiler_data.rendering_driver_type));
  ImGui::Text("Frame Time: %f ms", profiler_data.rendering_frame_time * 1000);
  ImGui::Text("Framerate: %u FPS", profiler_data.rendering_frame_rate);
#ifdef COMET_DEBUG_RENDERING
  ImGui::Text("Draw count: %u", profiler_data.rendering_draw_count);
#endif  // COMET_DEBUG_RENDERING
  ImGui::Unindent();
}

void DebuggerDisplayerManager::DrawMemorySection(
    const profiler::ProfilerData& profiler_data) const {
  allocation_tracker_displayer_.Draw(profiler_data);
}

void DebuggerDisplayerManager::DrawProfilingSection(
    const profiler::ProfilerData& profiler_data) {
  cpu_profiler_displayer_.Draw(profiler_data);
}
#endif
// COMET_IMGUI
#endif  // COMET_PROFILING
}  // namespace rendering
}  // namespace comet
#endif  //  COMET_DEBUG