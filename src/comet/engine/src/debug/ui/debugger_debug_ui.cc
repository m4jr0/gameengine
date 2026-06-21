// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_engine_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/engine/debug/ui/debugger_debug_ui.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_HAS_PROFILER_DEBUG_UI

#ifdef COMET_IMGUI
#include "imgui.h"
#endif  // COMET_IMGUI

#include "comet/data/resource/label/common_label.h"
#include "comet/platform/window/window_common.h"

namespace comet {
namespace debug {
void DebuggerDebugUi::Draw(const profiler::ProfilerData& profiler_data,
                           const CpuProfilerGraph::Controls& controls) {
  ImGui::SetNextWindowSizeConstraints(ImVec2{450.0f, 300.0f},
                                      ImVec2{FLT_MAX, FLT_MAX});

  ImGui::Begin("Mini Profiler");

  DrawSessionSection(profiler_data);
  ImGui::Spacing();

  DrawEntitySection(profiler_data);
  ImGui::Spacing();

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

void DebuggerDebugUi::DrawSessionSection(
    const profiler::ProfilerData& profiler_data) const {
  ImGui::Text("SESSION");
  ImGui::Indent();
  ImGui::Text("Uptime: %s", profiler_data.uptime);
  ImGui::Unindent();
}

void DebuggerDebugUi::DrawEntitySection(
    const profiler::ProfilerData& profiler_data) const {
  ImGui::Text("ENTITY");
  ImGui::Indent();
  ImGui::Text("Count: %zu", profiler_data.entity_count);
  ImGui::Text("Capacity: %zu", profiler_data.entity_capacity);
  ImGui::Text("Pending: %zu", profiler_data.pending_entity_count);
  ImGui::Unindent();
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
}  // namespace debug
}  // namespace comet

#endif  // COMET_HAS_PROFILER_DEBUG_UI