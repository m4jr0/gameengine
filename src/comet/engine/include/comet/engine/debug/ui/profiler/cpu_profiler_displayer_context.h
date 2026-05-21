// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_ENGINE_DEBUG_UI_PROFILER_CPU_PROFILER_DISPLAYER_CONTEXT_H_
#define COMET_ENGINE_DEBUG_UI_PROFILER_CPU_PROFILER_DISPLAYER_CONTEXT_H_

#include "comet/core/essentials.h"

#ifdef COMET_HAS_PROFILER_DEBUG_UI

#include "comet/profiler/profiler.h"

namespace comet {
namespace debug {
struct CpuProfilerDisplayerContext {
  bool is_frame_focused{false};
  usize frame_index{kInvalidIndex};
  const profiler::FrameContexts* frame_contexts{nullptr};

  void Focus(usize index);
  void UnFocus();

  usize GetFrameIndex() const;
  const profiler::FrameProfilerContext* GetFrameContext() const;
  const profiler::FrameProfilerContext* GetFrameContext(usize index) const;
};
}  // namespace debug
}  // namespace comet

#endif  // COMET_HAS_PROFILER_DEBUG_UI

#endif  // COMET_ENGINE_DEBUG_UI_PROFILER_CPU_PROFILER_DISPLAYER_CONTEXT_H_