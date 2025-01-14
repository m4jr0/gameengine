// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "cpu_profiler_displayer_context.h"

#ifdef COMET_PROFILING
namespace comet {
namespace rendering {
void CpuProfilerDisplayerContext::Focus(usize index) {
  frame_index = index;
  is_frame_focused = true;
}

void CpuProfilerDisplayerContext::UnFocus() {
  this->frame_index = kInvalidIndex;
  is_frame_focused = false;
}

usize CpuProfilerDisplayerContext::GetFrameIndex() const {
  return is_frame_focused ? frame_index : frame_contexts->GetSize() - 1;
}

const profiler::FrameProfilerContext*
CpuProfilerDisplayerContext::GetFrameContext() const {
  return GetFrameContext(GetFrameIndex());
}

const profiler::FrameProfilerContext*
CpuProfilerDisplayerContext::GetFrameContext(usize index) const {
  if (index >= frame_contexts->GetSize()) {
    return nullptr;
  }

  auto& box{(*frame_contexts)[index]};
  return box.has_value() ? &box.value() : nullptr;
}
}  // namespace rendering
}  // namespace comet
#endif  // COMET_PROFILING
