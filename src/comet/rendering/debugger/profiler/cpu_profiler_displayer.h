// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DEBUGGER_PROFILER_CPU_PROFILER_DISPLAYER_H_
#define COMET_COMET_RENDERING_DEBUGGER_PROFILER_CPU_PROFILER_DISPLAYER_H_

#include "comet/core/essentials.h"

#ifdef COMET_PROFILING
#ifdef COMET_IMGUI
#include "comet/profiler/profiler.h"
#include "comet/rendering/debugger/profiler/cpu_profiler_displayer_context.h"
#include "comet/rendering/debugger/profiler/cpu_profiler_graph.h"
#include "comet/rendering/debugger/profiler/cpu_profiler_tree.h"

namespace comet {
namespace rendering {
class CpuProfilerDisplayer {
 public:
  CpuProfilerDisplayer() = default;
  CpuProfilerDisplayer(const CpuProfilerDisplayer&) = delete;
  CpuProfilerDisplayer(CpuProfilerDisplayer&&) = delete;
  CpuProfilerDisplayer& operator=(const CpuProfilerDisplayer&) = delete;
  CpuProfilerDisplayer& operator=(CpuProfilerDisplayer&&) = delete;
  virtual ~CpuProfilerDisplayer() = default;

  void Draw(const profiler::ProfilerData& profiler_data);

 private:
  CpuProfilerDisplayerContext context_{};
  CpuProfilerGraph graph_{};
  CpuProfilerTree tree_{};
};
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI
#endif  // COMET_PROFILING

#endif  // COMET_COMET_RENDERING_DEBUGGER_PROFILER_CPU_PROFILER_DISPLAYER_H_
