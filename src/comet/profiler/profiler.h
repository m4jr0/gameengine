// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_PROFILER_PROFILER_H_
#define COMET_COMET_PROFILER_PROFILER_H_

#include <memory>
#include <stack>

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/concurrency/thread/thread.h"
#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/map.h"

#ifdef COMET_PROFILING
namespace comet {
namespace profiler {
constexpr usize kMaxProfileLabelLen{63};
using ProfileTimestamp = u64;

struct ProfilerNode {
  schar label[kMaxProfileLabelLen + 1]{'\0'};
  ProfileTimestamp start_time{0};
  ProfileTimestamp end_time{0};
  Array<ProfilerNode*> children{};

  ProfilerNode(memory::Allocator* allocator, const schar* label,
               ProfileTimestamp start_time);
};

struct ThreadProfilerContext {
  thread::ThreadId thread_id{thread::kInvalidThreadId};
  std::stack<ProfilerNode*> active_nodes{};
  Array<std::unique_ptr<ProfilerNode>> root_nodes{};

  ThreadProfilerContext(memory::Allocator* allocator = nullptr);
};

struct FiberProfilerContext {
  std::stack<ProfilerNode*> active_nodes{};
  Array<std::unique_ptr<ProfilerNode>> root_nodes{};

  FiberProfilerContext(memory::Allocator* allocator = nullptr);
};

struct FrameProfilerContext {
  frame::FrameCount frame_count{0};
  Map<fiber::FiberId, FiberProfilerContext*> fiber_contexts{};
  Map<thread::ThreadId, ThreadProfilerContext*> thread_contexts{};

  FrameProfilerContext(memory::Allocator* allocator = nullptr);
};

class ProfiledScope {
 public:
  explicit ProfiledScope(const schar* label);
  ~ProfiledScope();
};
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING

#ifdef COMET_PROFILING
#define COMET_PROFILE(label) \
  comet::profiler::ProfiledScope profiler { label }
#else
#define COMET_PROFILE(label)
#endif  // COMET_PROFILING

#endif  // COMET_COMET_PROFILER_PROFILER_H_
