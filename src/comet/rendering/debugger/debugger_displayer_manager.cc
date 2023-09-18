// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "debugger_displayer_manager.h"

#include "comet/core/memory/memory.h"

#ifdef COMET_DEBUG
#ifdef COMET_IMGUI
#include "imgui.h"
#endif  // COMET_IMGUI

namespace comet {
namespace rendering {
DebuggerDisplayerManager& DebuggerDisplayerManager::Get() {
  static DebuggerDisplayerManager singleton{};
  return singleton;
}

void DebuggerDisplayerManager::Update(const MiniProfilerPacket& packet) {
  mini_profiler_packet_ = packet;
}

void DebuggerDisplayerManager::Draw() {
#ifdef COMET_IMGUI
  constexpr auto kBufferCapacity{512};
  schar buffer[kBufferCapacity];
  uindex buffer_len;

  ImGui::Begin("Mini Profiler");

  // Physics.
  ImGui::Text("PHYSICS");
  ImGui::Indent();
  ImGui::Text("Frame Time: %f ms", mini_profiler_packet_.physics_frame_time);
  ImGui::Text("Framerate: %u Hz", mini_profiler_packet_.physics_frame_rate);
  ImGui::Unindent();

  // Rendering.
  ImGui::Spacing();
  ImGui::Text("RENDERING");
  ImGui::Indent();
  ImGui::Text("Driver: %s",
              GetDriverTypeLabel(mini_profiler_packet_.rendering_driver_type));
  ImGui::Text("Frame Time: %f ms", mini_profiler_packet_.rendering_frame_time);
  ImGui::Text("Framerate: %u FPS", mini_profiler_packet_.rendering_frame_rate);
  ImGui::Text("Draw count: %u", mini_profiler_packet_.rendering_draw_count);
  ImGui::Unindent();

  // Memory.
  GetMemorySizeString(mini_profiler_packet_.memory_use, buffer, kBufferCapacity,
                      &buffer_len);

  ImGui::Spacing();
  ImGui::Text("MEMORY");
  ImGui::Indent();
  ImGui::Text("Usage: %s", buffer);

  ImGui::End();
#endif  // COMET_IMGUI
}
}  // namespace rendering
}  // namespace comet
#endif  // COMET_DEBUG
