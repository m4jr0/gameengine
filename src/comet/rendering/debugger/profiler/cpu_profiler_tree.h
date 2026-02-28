// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DEBUGGER_PROFILER_CPU_PROFILER_TREE_H_
#define COMET_COMET_RENDERING_DEBUGGER_PROFILER_CPU_PROFILER_TREE_H_

#include "comet/core/essentials.h"

#ifdef COMET_PROFILING
#ifdef COMET_IMGUI
#include "comet/rendering/debugger/profiler/cpu_profiler_displayer_context.h"

namespace comet {
namespace rendering {
class CpuProfilerTree {
 public:
  CpuProfilerTree() = default;
  CpuProfilerTree(const CpuProfilerTree&) = delete;
  CpuProfilerTree(CpuProfilerTree&&) = delete;
  CpuProfilerTree& operator=(const CpuProfilerTree&) = delete;
  CpuProfilerTree& operator=(CpuProfilerTree&&) = delete;
  virtual ~CpuProfilerTree() = default;

  void Draw(const CpuProfilerDisplayerContext& context);

 private:
  void DrawProfilerNode(const profiler::ProfilerNode* node);
};
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI
#endif  // COMET_PROFILING

#endif  // COMET_COMET_RENDERING_DEBUGGER_PROFILER_CPU_PROFILER_TREE_H_
