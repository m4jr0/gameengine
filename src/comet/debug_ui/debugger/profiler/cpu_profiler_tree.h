// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_DEBUG_UI_DEBUGGER_PROFILER_CPU_PROFILER_TREE_H_
#define COMET_COMET_DEBUG_UI_DEBUGGER_PROFILER_CPU_PROFILER_TREE_H_

#include "comet/core/essentials.h"

#ifdef COMET_HAS_PROFILER_DEBUG_UI

#include "comet/debug_ui/debugger/profiler/cpu_profiler_displayer_context.h"

namespace comet {
namespace debugui {
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
  void DrawProfilerNode(const profiler::ProfilerNode* node) const;
};
}  // namespace debugui
}  // namespace comet

#endif  // COMET_HAS_PROFILER_DEBUG_UI

#endif  // COMET_COMET_DEBUG_UI_DEBUGGER_PROFILER_CPU_PROFILER_TREE_H_