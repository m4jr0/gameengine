// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "profiler.h"

#include "comet/core/c_string.h"
#include "comet/core/memory/memory_utils.h"
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

ProfilerNode::ProfilerNode(ProfilerNode&& other) noexcept
    : start_time{other.start_time},
      end_time{other.end_time},
      children{std::move(other.children)} {
  memory::CopyMemory(label, other.label, sizeof(label));
  other.start_time = 0;
  other.end_time = 0;
  other.label[0] = '\0';
}

ThreadProfilerContext::ThreadProfilerContext(memory::Allocator* allocator)
    : root_nodes{allocator} {}

ThreadProfilerContext::ThreadProfilerContext(
    ThreadProfilerContext&& other) noexcept
    : thread_id{other.thread_id},
      active_nodes{std::move(other.active_nodes)},
      root_nodes{std::move(other.root_nodes)} {
  other.thread_id = thread::kInvalidThreadId;
}

FiberProfilerContext::FiberProfilerContext(memory::Allocator* allocator)
    : root_nodes{allocator} {}

FrameProfilerContext::FrameProfilerContext(memory::Allocator* allocator)
    : fiber_contexts{allocator}, thread_contexts{allocator} {}

ProfilerRecordContext::ProfilerRecordContext(memory::Allocator* allocator)
    : frame_contexts{allocator} {}

ProfiledScope::ProfiledScope(const schar* label) {
  ProfilerManager::Get().StartProfiling(label);
}

ProfiledScope::~ProfiledScope() { ProfilerManager::Get().StopProfiling(); }
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING
