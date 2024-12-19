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

  ImGui::Spacing();
  DrawProfilingSection(profiler_data);

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

void DebuggerDisplayerManager::DrawProfilingSection(
    const profiler::ProfilerData& profiler_data) {
  ImGui::Text("PROFILING");
  auto is_recording{profiler_data.record_context.is_recording};
  auto& profiler_manager{profiler::ProfilerManager::Get()};

  if (ImGui::Button(is_recording ? "Stop Recording" : "Record")) {
    profiler_manager.ToggleRecording();
  }

  auto& record_context{profiler_data.record_context};
  auto& frame_contexts{record_context.frame_contexts};

  if (frame_contexts.IsEmpty()) {
    ImGui::Text("No recorded data to visualize.");
    return;
  }

  if (ImGui::Button("<< Previous Frame")) {
    NavigateFrame(record_context, -1);
  }

  ImGui::SameLine();

  if (ImGui::Button("Next Frame >>")) {
    NavigateFrame(record_context, 1);
  }

  auto& context{is_frame_frozen_
                    ? record_context.frame_contexts[frozen_frame_index_]
                    : record_context.frame_contexts.GetLast()};

  ImGui::Text("Frame #%llu", context.frame_count);

  for (auto& pair : context.thread_contexts) {
    const auto& thread_context{pair.value};

    if (ImGui::TreeNode(reinterpret_cast<void*>(thread_context.thread_id),
                        "Thread #%zu", thread_context.thread_id)) {
      for (const auto& node : thread_context.root_nodes) {
        DrawProfilerNode(node.get());
      }

      ImGui::TreePop();
    }
  }
}

void DebuggerDisplayerManager::DrawProfilerNode(
    const profiler::ProfilerNode* node) {
  if (node == nullptr) {
    return;
  }

  if (ImGui::TreeNode(
          node, "%s (%.2f ms)", node->label,
          static_cast<f64>(node->end_time - node->start_time) / 1000000.0f)) {
    for (const auto* child : node->children) {
      DrawProfilerNode(child);
    }

    ImGui::TreePop();
  }
}

void DebuggerDisplayerManager::NavigateFrame(
    const profiler::ProfilerRecordContext& record_context, s8 direction) {
  auto frame_context_count{record_context.frame_contexts.GetSize()};

  // If there's only one frame, do nothing
  if (frame_context_count == 1) {
    return;
  }

  // If not frozen, initialize frozen state
  if (!is_frame_frozen_) {
    frozen_frame_index_ = frame_context_count - 1;
    is_frame_frozen_ = true;
  }

  // Navigate frames based on direction
  frozen_frame_index_ =
      (frozen_frame_index_ + direction + frame_context_count) %
      frame_context_count;
}

void DebuggerDisplayerManager::UnfreezeFrame() {
  is_frame_frozen_ = false;
  frozen_frame_index_ = 0;
}
#endif  // COMET_IMGUI
}  // namespace rendering
}  // namespace comet
#endif  //  COMET_PROFILING