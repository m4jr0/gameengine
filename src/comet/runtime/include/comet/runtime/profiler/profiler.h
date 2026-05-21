// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_PROFILER_PROFILER_H_
#define COMET_RUNTIME_PROFILER_PROFILER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
#include <optional>
#include <stack>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/container/array.h"
#include "comet/core/container/map.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/thread/thread.h"

#ifdef COMET_PROFILING
namespace comet {
namespace profiler {
constexpr usize kMaxProfileLabelLen{127};
using ProfilerTimestamp = u64;
using ProfilerElapsedTime = f32;
using ProfilerNodeId = usize;
constexpr auto kInvalidProfilerNodeId{static_cast<ProfilerNodeId>(-1)};

struct ProfilerNode {
  schar label[kMaxProfileLabelLen + 1]{'\0'};
  ProfilerElapsedTime elapsed_time_ms{.0f};
  ProfilerNodeId id{kInvalidProfilerNodeId};
  ProfilerTimestamp start_time{0};
  ProfilerTimestamp end_time{0};
  Array<ProfilerNode*> children{};

  ProfilerNode(memory::Allocator* allocator, const schar* label,
               ProfilerTimestamp start_time);
  ProfilerNode(const ProfilerNode& other);
  ProfilerNode(ProfilerNode&& other) noexcept;
  ProfilerNode& operator=(const ProfilerNode& other);
  ProfilerNode& operator=(ProfilerNode&& other) noexcept;
  ~ProfilerNode() = default;

 private:
  static inline std::atomic<ProfilerNodeId> id_counter{0};
};

struct ThreadProfilerContext {
  thread::ThreadId thread_id{thread::kInvalidThreadId};
  std::stack<ProfilerNode*> active_nodes{};
  Array<ProfilerNode*> root_nodes{};
  Array<memory::UniquePtr<ProfilerNode>> nodes{};

  explicit ThreadProfilerContext(memory::Allocator* allocator = nullptr);
  ThreadProfilerContext(const ThreadProfilerContext&) = delete;
  ThreadProfilerContext(ThreadProfilerContext&& other) noexcept;
  ThreadProfilerContext& operator=(const ThreadProfilerContext&) = delete;
  ThreadProfilerContext& operator=(ThreadProfilerContext&& other) noexcept;
  ~ThreadProfilerContext() = default;
};

struct FrameProfilerContext {
  ProfilerElapsedTime elapsed_time_ms{.0f};
  ProfilerTimestamp start_time{0};
  ProfilerTimestamp end_time{0};
  frame::FrameCount frame_count{0};
  Map<thread::ThreadId, ThreadProfilerContext> thread_contexts{};

  explicit FrameProfilerContext(memory::Allocator* allocator = nullptr);
  FrameProfilerContext(const FrameProfilerContext&) = delete;
  FrameProfilerContext(FrameProfilerContext&& other) noexcept;
  FrameProfilerContext& operator=(const FrameProfilerContext&) = delete;
  FrameProfilerContext& operator=(FrameProfilerContext&& other) noexcept;
  ~FrameProfilerContext() = default;
};

using FrameContexts = Array<std::optional<FrameProfilerContext>>;

struct ProfilerRecordContext {
  frame::FrameCount start_frame_count{0};
  frame::FrameCount end_frame_count{0};
  FrameContexts frame_contexts{};

  explicit ProfilerRecordContext(memory::Allocator* allocator = nullptr);
  ProfilerRecordContext(const ProfilerRecordContext&) = delete;
  ProfilerRecordContext(ProfilerRecordContext&& other) noexcept;
  ProfilerRecordContext& operator=(const ProfilerRecordContext&) = delete;
  ProfilerRecordContext& operator=(ProfilerRecordContext&& other) noexcept;
  ~ProfilerRecordContext() = default;
};

class ProfiledScope {
 public:
  explicit ProfiledScope(const schar* label);
  ProfiledScope(const ProfiledScope&) = default;
  ProfiledScope(ProfiledScope&&) noexcept = default;
  ProfiledScope& operator=(const ProfiledScope&) = default;
  ProfiledScope& operator=(ProfiledScope&&) noexcept = default;
  ~ProfiledScope();
};
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING

#ifdef COMET_PROFILING
#define COMET_PROFILE(label)                                                \
  comet::profiler::ProfiledScope COMET_CONCAT(_comet_profiler_, __LINE__) { \
    label                                                                   \
  }
#else
#define COMET_PROFILE(label) ((void)0)
#endif  // COMET_PROFILING

#endif  // COMET_RUNTIME_PROFILER_PROFILER_H_