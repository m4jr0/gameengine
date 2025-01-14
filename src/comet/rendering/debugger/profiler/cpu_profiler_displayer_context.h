// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DEBUGGER_PROFILER_CPU_PROFILER_DISPLAYER_CONTEXT_H_
#define COMET_COMET_RENDERING_DEBUGGER_PROFILER_CPU_PROFILER_DISPLAYER_CONTEXT_H_

#include "comet/core/essentials.h"

#ifdef COMET_PROFILING
#include <optional>

#include "comet/profiler/profiler.h"
namespace comet {
namespace rendering {
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
}  // namespace rendering
}  // namespace comet
#endif  // COMET_PROFILING

#endif  // COMET_COMET_RENDERING_DEBUGGER_PROFILER_CPU_PROFILER_DISPLAYER_CONTEXT_H_
