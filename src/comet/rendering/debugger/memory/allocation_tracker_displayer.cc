// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "allocation_tracker_displayer.h"

#ifdef COMET_PROFILING
#ifdef COMET_IMGUI
#include <algorithm>

#include "imgui.h"

#include "comet/core/frame/frame_manager.h"
#include "comet/core/memory/memory.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/rendering/debugger/imgui_utils.h"

namespace comet {
namespace rendering {
void AllocationTrackerDisplayer::Draw(
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
          frame::FrameManager::Get().GetFrameAllocator(),
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
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI
#endif  // COMET_PROFILING