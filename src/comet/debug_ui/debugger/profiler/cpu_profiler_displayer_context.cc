// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "cpu_profiler_displayer_context.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_HAS_PROFILER_DEBUG_UI

namespace comet {
namespace debugui {
void CpuProfilerDisplayerContext::Focus(usize index) {
  frame_index = index;
  is_frame_focused = true;
}

void CpuProfilerDisplayerContext::UnFocus() {
  frame_index = kInvalidIndex;
  is_frame_focused = false;
}

usize CpuProfilerDisplayerContext::GetFrameIndex() const {
  if (frame_contexts == nullptr || frame_contexts->IsEmpty()) {
    return kInvalidIndex;
  }

  if (is_frame_focused) {
    return frame_index;
  }

  return frame_contexts->GetSize() - 1;
}

const profiler::FrameProfilerContext*
CpuProfilerDisplayerContext::GetFrameContext() const {
  auto index{GetFrameIndex()};

  if (index == kInvalidIndex) {
    return nullptr;
  }

  return GetFrameContext(index);
}

const profiler::FrameProfilerContext*
CpuProfilerDisplayerContext::GetFrameContext(usize index) const {
  if (frame_contexts == nullptr || index >= frame_contexts->GetSize()) {
    return nullptr;
  }

  auto& box{(*frame_contexts)[index]};
  return box.has_value() ? &box.value() : nullptr;
}
}  // namespace debugui
}  // namespace comet

#endif  // COMET_HAS_PROFILER_DEBUG_UI