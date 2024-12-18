// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "profiler.h"

#include "comet/core/c_string.h"
#include "comet/math/math_commons.h"
#include "comet/profiler/profiler_manager.h"

#ifdef COMET_PROFILING
namespace comet {
namespace profiler {
ProfilerNode::ProfilerNode(memory::Allocator* allocator, const schar* label,
                           ProfileTimestamp start_time)
    : start_time{start_time}, children{allocator} {
  Copy(this->label, label, math::Min(GetLength(label), kMaxProfileLabelLen));
}

ThreadProfilerContext::ThreadProfilerContext(memory::Allocator* allocator)
    : root_nodes{allocator} {}

FiberProfilerContext::FiberProfilerContext(memory::Allocator* allocator)
    : root_nodes{allocator} {}

FrameProfilerContext::FrameProfilerContext(memory::Allocator* allocator)
    : fiber_contexts{allocator}, thread_contexts{allocator} {}

ProfiledScope::ProfiledScope(const schar* label) {
  ProfilerManager::Get().StartProfiling(label);
}

ProfiledScope::~ProfiledScope() { ProfilerManager::Get().StopProfiling(); }
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING
