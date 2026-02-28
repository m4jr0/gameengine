// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "profiler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/math/math_common.h"
#include "comet/profiler/profiler_manager.h"

#ifdef COMET_PROFILING
namespace comet {
namespace profiler {
ProfilerNode::ProfilerNode(memory::Allocator* allocator, const schar* label,
                           ProfilerTimestamp start_time)
    : id{id_counter.fetch_add(1, std::memory_order_acq_rel)},
      start_time{start_time},
      children{allocator} {
  this->label[0] = '#';
  usize out_len;
  ConvertToStr(id, this->label + 1, kMaxProfileLabelLen - 1, &out_len);
  this->label[1 + out_len] = ' ';
  Copy(this->label + 2 + out_len, label,
       math::Min(GetLength(label), kMaxProfileLabelLen - 2 - out_len));
}

ProfilerNode::ProfilerNode(const ProfilerNode& other)
    : elapsed_time_ms{other.elapsed_time_ms},
      id{other.id},
      start_time{other.start_time},
      end_time{other.end_time},
      children{other.children} {
  Copy(label, other.label, kMaxProfileLabelLen + 1);
}

ProfilerNode::ProfilerNode(ProfilerNode&& other) noexcept
    : elapsed_time_ms{other.elapsed_time_ms},
      id{other.id},
      start_time{other.start_time},
      end_time{other.end_time},
      children{std::move(other.children)} {
  Copy(label, other.label, kMaxProfileLabelLen + 1);

  Clear(other.label, kMaxProfileLabelLen + 1);
  other.elapsed_time_ms = .0f;
  other.id = kInvalidProfilerNodeId;
  other.start_time = 0;
  other.end_time = 0;
}

ProfilerNode& ProfilerNode::operator=(const ProfilerNode& other) {
  if (this == &other) {
    return *this;
  }

  Copy(label, other.label, kMaxProfileLabelLen + 1);
  elapsed_time_ms = other.elapsed_time_ms;
  id = other.id;
  start_time = other.start_time;
  end_time = other.end_time;
  children = other.children;

  return *this;
}

ProfilerNode& ProfilerNode::operator=(ProfilerNode&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Copy(label, other.label, kMaxProfileLabelLen + 1);
  elapsed_time_ms = other.elapsed_time_ms;
  id = other.id;
  start_time = other.start_time;
  end_time = other.end_time;
  children = std::move(other.children);

  Clear(other.label, kMaxProfileLabelLen + 1);
  other.elapsed_time_ms = .0f;
  other.id = kInvalidProfilerNodeId;
  other.start_time = 0;
  other.end_time = 0;

  return *this;
}

ThreadProfilerContext::ThreadProfilerContext(memory::Allocator* allocator)
    : root_nodes{allocator}, nodes{allocator} {}

ThreadProfilerContext::ThreadProfilerContext(
    ThreadProfilerContext&& other) noexcept
    : thread_id{other.thread_id},
      active_nodes{std::move(other.active_nodes)},
      root_nodes{std::move(other.root_nodes)},
      nodes{std::move(other.nodes)} {
  other.thread_id = thread::kInvalidThreadId;
}

ThreadProfilerContext& ThreadProfilerContext::operator=(
    ThreadProfilerContext&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  thread_id = other.thread_id;
  active_nodes = std::move(other.active_nodes);
  root_nodes = std::move(other.root_nodes);
  nodes = std::move(other.nodes);

  other.thread_id = thread::kInvalidThreadId;
  return *this;
}

FrameProfilerContext::FrameProfilerContext(memory::Allocator* allocator)
    : thread_contexts{allocator} {}

FrameProfilerContext::FrameProfilerContext(
    FrameProfilerContext&& other) noexcept
    : elapsed_time_ms{other.elapsed_time_ms},
      start_time{other.start_time},
      end_time{other.end_time},
      frame_count{other.frame_count},
      thread_contexts{std::move(other.thread_contexts)} {
  other.elapsed_time_ms = .0f;
  other.start_time = 0;
  other.end_time = 0;
  other.frame_count = 0;
}

FrameProfilerContext& FrameProfilerContext::operator=(
    FrameProfilerContext&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  elapsed_time_ms = other.elapsed_time_ms;
  start_time = other.start_time;
  end_time = other.end_time;
  frame_count = other.frame_count;
  thread_contexts = std::move(other.thread_contexts);

  other.elapsed_time_ms = .0f;
  other.start_time = 0;
  other.end_time = 0;
  other.frame_count = 0;
  return *this;
}

ProfilerRecordContext::ProfilerRecordContext(memory::Allocator* allocator)
    : frame_contexts{allocator} {}

ProfilerRecordContext::ProfilerRecordContext(
    ProfilerRecordContext&& other) noexcept
    : start_frame_count{other.start_frame_count},
      end_frame_count{other.end_frame_count},
      frame_contexts{std::move(other.frame_contexts)} {
  other.start_frame_count = 0;
  other.end_frame_count = 0;
}

ProfilerRecordContext& ProfilerRecordContext::operator=(
    ProfilerRecordContext&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  start_frame_count = other.start_frame_count;
  end_frame_count = other.end_frame_count;
  frame_contexts = std::move(other.frame_contexts);

  other.start_frame_count = 0;
  other.end_frame_count = 0;
  return *this;
}

ProfilerData::ProfilerData(memory::Allocator* allocator)
    : record_context{allocator} {}

ProfilerData::ProfilerData(ProfilerData&& other) noexcept
    : physics_frame_time{other.physics_frame_time},
      physics_frame_rate{other.physics_frame_rate},
      rendering_frame_time{other.rendering_frame_time},
      rendering_frame_rate{other.rendering_frame_rate},
#ifdef COMET_DEBUG_RENDERING
      rendering_draw_count{other.rendering_draw_count},
#endif  // COMET_DEBUG_RENDERING
      rendering_driver_type{other.rendering_driver_type},
      memory_use{other.memory_use},
      tag_use{std::move(other.tag_use)},
      record_context{std::move(other.record_context)} {
  other.physics_frame_rate = 0;
  other.rendering_frame_time = 0;
  other.rendering_frame_rate = 0;
#ifdef COMET_DEBUG_RENDERING
  other.rendering_draw_count = 0;
#endif  // COMET_DEBUG_RENDERING
  other.rendering_driver_type = rendering::DriverType::Unknown;
  other.memory_use = 0;
}

ProfilerData& ProfilerData::operator=(ProfilerData&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  physics_frame_rate = other.physics_frame_rate;
  rendering_frame_time = other.rendering_frame_time;
  rendering_frame_rate = other.rendering_frame_rate;
#ifdef COMET_DEBUG_RENDERING
  rendering_draw_count = other.rendering_draw_count;
#endif  // COMET_DEBUG_RENDERING
  rendering_driver_type = other.rendering_driver_type;
  memory_use = other.memory_use;
  tag_use = std::move(other.tag_use);
  record_context = std::move(other.record_context);

  other.physics_frame_rate = 0;
  other.rendering_frame_time = 0;
  other.rendering_frame_rate = 0;
#ifdef COMET_DEBUG_RENDERING
  other.rendering_draw_count = 0;
#endif  // COMET_DEBUG_RENDERING
  other.rendering_driver_type = rendering::DriverType::Unknown;
  other.memory_use = 0;
  return *this;
}

ProfiledScope::ProfiledScope(const schar* label) {
  ProfilerManager::Get().StartProfiling(label);
}

ProfiledScope::~ProfiledScope() { ProfilerManager::Get().StopProfiling(); }
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING
