// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "allocation_tracker_displayer.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_HAS_MEMORY_DEBUG_UI

// External. ///////////////////////////////////////////////////////////////////
#include <algorithm>

#include "imgui.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_manager.h"
#include "comet/core/memory/memory.h"
#include "comet/core/memory/memory_label.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/debug_ui/imgui_utils.h"

namespace comet {
namespace debugui {
void AllocationTrackerDisplayer::Draw(
    const profiler::ProfilerData& profiler_data) const {
  constexpr usize kBufferCapacity{512};
  schar buffer[kBufferCapacity];
  usize buffer_len{0};

  memory::GetMemorySizeString(profiler_data.memory_use, buffer, kBufferCapacity,
                              &buffer_len);

  ImGui::Text("MEMORY");
  ImGui::Indent();

  ImGui::Text("Usage: %s", buffer);

  if (ImGui::CollapsingHeader("Tags")) {
    if (ImGui::BeginTable("MemoryTagsTable", 2,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("Tag", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();

      auto sorted_tags{Array<Pair<memory::MemoryTag, usize>>::WithCapacity(
          frame::FrameManager::Get().GetFrameAllocator(),
          profiler_data.tag_use.GetEntryCount())};

      for (const auto& pair : profiler_data.tag_use) {
        sorted_tags.PushLast(pair);
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
}  // namespace debugui
}  // namespace comet

#endif  // COMET_HAS_MEMORY_DEBUG_UI