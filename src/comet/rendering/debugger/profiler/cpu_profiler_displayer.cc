// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "cpu_profiler_displayer.h"

#ifdef COMET_PROFILING
#ifdef COMET_IMGUI
namespace comet {
namespace rendering {
void CpuProfilerDisplayer::Draw(const profiler::ProfilerData& profiler_data) {
  context_.frame_contexts = &profiler_data.record_context.frame_contexts;
  ImGui::Text("PROFILING");
  graph_.Draw(context_);
  tree_.Draw(context_);
}
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI
#endif  // COMET_PROFILING
