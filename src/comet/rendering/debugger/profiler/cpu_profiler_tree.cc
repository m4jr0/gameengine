// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "cpu_profiler_tree.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_PROFILING
#ifdef COMET_IMGUI
// External. ///////////////////////////////////////////////////////////////////
#include "imgui.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/profiler/profiler.h"

namespace comet {
namespace rendering {
void CpuProfilerTree::Draw(const CpuProfilerDisplayerContext& context) {
  const auto& frame_contexts{*context.frame_contexts};

  if (frame_contexts.IsEmpty()) {
    ImGui::Text("No recorded data to visualize.");
    return;
  }

  if (ImGui::BeginChild("ProfilerData")) {
    const auto* frame_context{context.GetFrameContext()};

    if (frame_context == nullptr) {
      ImGui::Text("No recorded data to visualize.");
      ImGui::EndChild();
      return;
    }

    ImGui::Text("Frame #%zu | %.2f ms", frame_context->frame_count,
                frame_context->elapsed_time_ms);

    for (auto& pair : frame_context->thread_contexts) {
      const auto& thread_context{pair.value};
      auto are_children{!thread_context.root_nodes.IsEmpty()};

      if (ImGui::TreeNodeEx(reinterpret_cast<void*>(thread_context.thread_id),
                            !are_children
                                ? ImGuiTreeNodeFlags_Leaf |
                                      ImGuiTreeNodeFlags_NoTreePushOnOpen
                                : 0,
                            "Thread #%zu", thread_context.thread_id)) {
        for (const auto* node : thread_context.root_nodes) {
          DrawProfilerNode(node);
        }

        if (are_children) {
          ImGui::TreePop();
        }
      }
    }
  }

  ImGui::EndChild();
}

void CpuProfilerTree::DrawProfilerNode(const profiler::ProfilerNode* node) {
  if (node == nullptr) {
    return;
  }

  auto are_children{!node->children.IsEmpty()};

  if (ImGui::TreeNodeEx(node,
                        are_children ? 0
                                     : ImGuiTreeNodeFlags_Leaf |
                                           ImGuiTreeNodeFlags_NoTreePushOnOpen,
                        "%s (%.2f ms)", node->label, node->elapsed_time_ms)) {
    for (const auto* child : node->children) {
      DrawProfilerNode(child);
    }

    if (are_children) {
      ImGui::TreePop();
    }
  }
}
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI
#endif  // COMET_PROFILING
