// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "debugger_debug_ui.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_HAS_PROFILER_DEBUG_UI

#ifdef COMET_IMGUI
#include "imgui.h"
#endif  // COMET_IMGUI

#include "comet/rendering/rendering_utils.h"

namespace comet {
namespace debugui {
void DebuggerDebugUi::Draw(const profiler::ProfilerData& profiler_data,
                           const CpuProfilerGraph::Controls& controls) {
  ImGui::SetNextWindowSizeConstraints(ImVec2{450.0f, 300.0f},
                                      ImVec2{FLT_MAX, FLT_MAX});

  ImGui::Begin("Mini Profiler");

  DrawPhysicsSection(profiler_data);
  ImGui::Spacing();

  DrawRenderingSection(profiler_data);

#ifdef COMET_HAS_MEMORY_DEBUG_UI
  ImGui::Spacing();
  DrawMemorySection(profiler_data);
#endif  // COMET_HAS_MEMORY_DEBUG_UI

  ImGui::Spacing();
  DrawProfilingSection(profiler_data, controls);

  ImGui::End();
}

void DebuggerDebugUi::DrawPhysicsSection(
    const profiler::ProfilerData& profiler_data) const {
  ImGui::Text("PHYSICS");
  ImGui::Indent();
  ImGui::Text("Frame Time: %f ms", profiler_data.physics_frame_time * 1000.0f);
  ImGui::Text("Framerate: %u Hz", profiler_data.physics_frame_rate);
  ImGui::Unindent();
}

void DebuggerDebugUi::DrawRenderingSection(
    const profiler::ProfilerData& profiler_data) const {
  ImGui::Text("RENDERING");
  ImGui::Indent();
  ImGui::Text("Driver: %s",
              GetDriverTypeLabel(profiler_data.rendering_driver_type));
  ImGui::Text("Frame Time: %f ms",
              profiler_data.rendering_frame_time * 1000.0f);
  ImGui::Text("Framerate: %u FPS", profiler_data.rendering_frame_rate);

#ifdef COMET_DEBUG_RENDERING
  ImGui::Text("Draw count: %u", profiler_data.rendering_draw_count);
#endif  // COMET_DEBUG_RENDERING

  ImGui::Unindent();
}

#ifdef COMET_HAS_MEMORY_DEBUG_UI
void DebuggerDebugUi::DrawMemorySection(
    const profiler::ProfilerData& profiler_data) const {
  allocation_tracker_displayer_.Draw(profiler_data);
}
#endif  // COMET_HAS_MEMORY_DEBUG_UI

void DebuggerDebugUi::DrawProfilingSection(
    const profiler::ProfilerData& profiler_data,
    const CpuProfilerGraph::Controls& controls) {
  cpu_profiler_displayer_.Draw(profiler_data, controls);
}
}  // namespace debugui
}  // namespace comet

#endif  // COMET_HAS_PROFILER_DEBUG_UI