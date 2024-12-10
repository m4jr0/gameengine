// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "debugger_displayer_manager.h"

#ifdef COMET_PROFILING
#include <algorithm>

#include "comet/core/frame/frame_manager.h"
#include "comet/core/memory/memory.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/profiler/profiler_manager.h"

#ifdef COMET_IMGUI
#include "imgui.h"

#include "comet/rendering/debugger/imgui_utils.h"
#endif  // COMET_IMGUI

namespace comet {
namespace rendering {
DebuggerDisplayerManager& DebuggerDisplayerManager::Get() {
  static DebuggerDisplayerManager singleton{};
  return singleton;
}

void DebuggerDisplayerManager::Draw() {
#ifdef COMET_IMGUI
  auto& profiler_data{profiler::ProfilerManager::Get().GetData()};

  ImGui::Begin("Mini Profiler");

  DrawPhysicsSection(profiler_data);
  ImGui::Spacing();
  DrawRenderingSection(profiler_data);

#ifdef COMET_TRACK_ALLOCATIONS
  ImGui::Spacing();
  DrawMemorySection(profiler_data);
#endif  // COMET_TRACK_ALLOCATIONS

  ImGui::End();
#endif  // COMET_IMGUI
}

#ifdef COMET_IMGUI
void DebuggerDisplayerManager::DrawPhysicsSection(
    const profiler::ProfilerData& profiler_data) const {
  ImGui::Text("PHYSICS");
  ImGui::Indent();
  ImGui::Text("Frame Time: %f ms", profiler_data.physics_frame_time);
  ImGui::Text("Framerate: %u Hz", profiler_data.physics_frame_rate);
  ImGui::Unindent();
}

void DebuggerDisplayerManager::DrawRenderingSection(
    const profiler::ProfilerData& profiler_data) const {
  ImGui::Text("RENDERING");
  ImGui::Indent();
  ImGui::Text("Driver: %s",
              GetDriverTypeLabel(profiler_data.rendering_driver_type));
  ImGui::Text("Frame Time: %f ms", profiler_data.rendering_frame_time);
  ImGui::Text("Framerate: %u FPS", profiler_data.rendering_frame_rate);
  ImGui::Text("Draw count: %u", profiler_data.rendering_draw_count);
  ImGui::Unindent();
}

void DebuggerDisplayerManager::DrawMemorySection(
    const profiler::ProfilerData& profiler_data) const {
  constexpr auto kBufferCapacity{512};
  schar buffer[kBufferCapacity];
  usize buffer_len;

  memory::GetMemorySizeString(profiler_data.memory_use, buffer, kBufferCapacity,
                              &buffer_len);

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

      Array<Pair<memory::MemoryTag, usize>> sorted_tags{
          frame::FrameManager::Get().GetFrameAllocatorHandle()->allocator,
          profiler_data.tag_use.GetEntryCount()};

      for (auto& pair : profiler_data.tag_use) {
        sorted_tags.PushBack(pair);
      }

      std::sort(
          sorted_tags.begin(), sorted_tags.end(),
          [](const auto& lhs, const auto& rhs) { return lhs.key < rhs.key; });

      usize total_size{0};

      AddTableEntries(
          sorted_tags,
          [](const auto& entry) -> const schar* {
            return memory::GetMemoryTagLabel(entry.key);
          },
          [&](const auto& entry) {
            memory::GetMemorySizeString(entry.value, buffer, kBufferCapacity,
                                        &buffer_len);

            if (entry.key != memory::kEngineMemoryTagRenderingDevice) {
              total_size += entry.value;
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
            static_cast<ssize>(profiler_data.memory_use) -
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
#endif  //  COMET_PROFILING