// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "debugger_displayer_manager.h"

#include <algorithm>

#include "comet/core/memory/memory.h"

#ifdef COMET_DEBUG
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

void DebuggerDisplayerManager::Update(const MiniProfilerPacket& packet) {
  mini_profiler_packet_ = packet;
}

void DebuggerDisplayerManager::Draw() {
#ifdef COMET_IMGUI
  ImGui::Begin("Mini Profiler");

  DrawPhysicsSection();
  ImGui::Spacing();
  DrawRenderingSection();

#ifdef COMET_TRACK_ALLOCATIONS
  ImGui::Spacing();
  DrawMemorySection();
#endif  // COMET_TRACK_ALLOCATIONS

  ImGui::End();
#endif  // COMET_IMGUI
}

#ifdef COMET_IMGUI
void DebuggerDisplayerManager::DrawPhysicsSection() const {
  ImGui::Text("PHYSICS");
  ImGui::Indent();
  ImGui::Text("Frame Time: %f ms", mini_profiler_packet_.physics_frame_time);
  ImGui::Text("Framerate: %u Hz", mini_profiler_packet_.physics_frame_rate);
  ImGui::Unindent();
}

void DebuggerDisplayerManager::DrawRenderingSection() const {
  ImGui::Text("RENDERING");
  ImGui::Indent();
  ImGui::Text("Driver: %s",
              GetDriverTypeLabel(mini_profiler_packet_.rendering_driver_type));
  ImGui::Text("Frame Time: %f ms", mini_profiler_packet_.rendering_frame_time);
  ImGui::Text("Framerate: %u FPS", mini_profiler_packet_.rendering_frame_rate);
  ImGui::Text("Draw count: %u", mini_profiler_packet_.rendering_draw_count);
  ImGui::Unindent();
}

void DebuggerDisplayerManager::DrawMemorySection() const {
  constexpr auto kBufferCapacity{512};
  schar buffer[kBufferCapacity];
  usize buffer_len;

  memory::GetMemorySizeString(mini_profiler_packet_.memory_use, buffer,
                              kBufferCapacity, &buffer_len);

  ImGui::Text("MEMORY");
  ImGui::Indent();

#ifdef COMET_MSVC
  ImGui::Text("Usage: %s [Unreliable]", buffer);
#else
  ImGui::Text("Usage: %s [Unavailable]", buffer);
#endif  // COMET_MSVC

  if (ImGui::CollapsingHeader("Tags")) {
    if (ImGui::BeginTable("MemoryTagsTable", 2,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("Tag", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();

      std::vector<std::pair<memory::MemoryTag, usize>> sorted_tags{
          mini_profiler_packet_.tag_use.begin(),
          mini_profiler_packet_.tag_use.end()};

      std::sort(sorted_tags.begin(), sorted_tags.end(),
                [](const auto& lhs, const auto& rhs) {
                  return lhs.first < rhs.first;
                });

      usize total_size{0};

      AddTableEntries(
          sorted_tags,
          [](const auto& entry) -> const schar* {
            return memory::GetMemoryTagLabel(entry.first);
          },
          [&](const auto& entry) {
            memory::GetMemorySizeString(entry.second, buffer, kBufferCapacity,
                                        &buffer_len);

            if (entry.first != memory::kEngineMemoryTagRenderingDevice) {
              total_size += entry.second;
            }

            ImGui::Text("%s", buffer);
          });

      AddTableRow("TOTAL", [&]() -> const schar* {
        memory::GetMemorySizeString(total_size, buffer, kBufferCapacity,
                                    &buffer_len);
        return buffer;
      }());

      AddTableRow("UNTRACKED", [&]() -> const schar* {
        memory::GetMemorySizeString(
            static_cast<ssize>(mini_profiler_packet_.memory_use) -
                static_cast<ssize>(total_size),
            buffer, kBufferCapacity, &buffer_len);
        return buffer;
      }());

      ImGui::EndTable();
    }
  }

  ImGui::Unindent();
}
#endif  // COMET_IMGUI
}  // namespace rendering
}  // namespace comet
#endif  // COMET_DEBUG
