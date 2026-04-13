// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "cpu_profiler_displayer.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_HAS_PROFILER_DEBUG_UI

// External. ///////////////////////////////////////////////////////////////////
#include "imgui.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace debugui {
void CpuProfilerDisplayer::Draw(const profiler::ProfilerData& profiler_data,
                                const CpuProfilerGraph::Controls& controls) {
  context_.frame_contexts = &profiler_data.record_context.frame_contexts;

  ImGui::Text("PROFILING");

  if (context_.frame_contexts == nullptr ||
      context_.frame_contexts->IsEmpty()) {
    ImGui::Text("No recorded data to visualize.");
    return;
  }

  graph_.Draw(context_, controls);
  tree_.Draw(context_);
}
}  // namespace debugui
}  // namespace comet

#endif  // COMET_HAS_PROFILER_DEBUG_UI