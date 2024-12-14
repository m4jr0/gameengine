// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "profiler_manager.h"

#include "comet/core/date.h"
#include "comet/core/game_state_manager.h"
#include "comet/core/memory/allocation_tracking.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/rendering_manager.h"

#ifdef COMET_PROFILING
namespace comet {
namespace profiler {
ProfilerManager& ProfilerManager::Get() {
  static ProfilerManager singleton{};
  return singleton;
}

void ProfilerManager::Initialize() {
  Manager::Initialize();
  thread_contexts_.Initialize();
  allocator_.Initialize();
  auto thread_context_count{thread_contexts_.GetSize()};

  for (thread::ThreadId i{0}; i < thread_context_count; ++i) {
    auto& thread_context{thread_contexts_.GetFromIndex(i)};
    thread_context.thread_id = thread_contexts_.GetThreadIdFromIndex(i);
  }
}

void ProfilerManager::Shutdown() {
  thread_contexts_.Destroy();
  allocator_.Destroy();
  Manager::Shutdown();
}

void ProfilerManager::Update() {
#ifdef COMET_DEBUG
  auto& physics_manager{physics::PhysicsManager::Get()};
  auto& rendering_manager{rendering::RenderingManager::Get()};

  data_.physics_frame_time = physics_manager.GetFrameTime();
  data_.physics_frame_rate = physics_manager.GetFrameRate();
  data_.rendering_driver_type = rendering_manager.GetDriverType();
  data_.rendering_frame_time = rendering_manager.GetFrameTime();
  data_.rendering_frame_rate = rendering_manager.GetFrameRate();
  data_.rendering_draw_count = rendering_manager.GetDrawCount();
  COMET_GET_MEMORY_USE(data_.memory_use);
  COMET_GET_TAG_USE(data_.tag_use);
#endif  // COMET_DEBUG
}

void ProfilerManager::StartFrame(frame::FrameCount frame_count) {
  if (!is_recording_) {
    return;
  }

  recording_frame_context_ = {&allocator_};
  recording_frame_context_.start_time = GetTimestampNanoSeconds();
  recording_frame_context_.frame_count = frame_count;
  is_frame_recording_ = true;
}

void ProfilerManager::EndFrame() {
  RecordFrame();
  auto thread_context_count{thread_contexts_.GetSize()};

  for (usize i{0}; i < thread_context_count; ++i) {
    auto& thread_context{thread_contexts_.GetFromIndex(i)};
    thread_context.thread_id = thread_contexts_.GetThreadIdFromIndex(i);
    thread_context.active_nodes = {};
    thread_context.root_nodes = Array<ProfilerNode*>{&allocator_};
    thread_context.nodes = Array<std::unique_ptr<ProfilerNode>>{&allocator_};
  }
}

void ProfilerManager::StartProfiling(const schar* label) {
  if (!is_recording_) {
    return;
  }

  auto& thread_context{thread_contexts_.Get()};
  auto now{GetTimestampNanoSeconds()};
  auto node{std::make_unique<ProfilerNode>(&allocator_, label, now)};

  if (thread_context.active_nodes.empty()) {
    thread_context.root_nodes.PushBack(node.get());
  } else {
    thread_context.active_nodes.top()->children.PushBack(node.get());
  }

  thread_context.active_nodes.push(node.get());
  thread_context.nodes.PushBack(std::move(node));
}

void ProfilerManager::StopProfiling() {
  if (!is_recording_) {
    return;
  }

  auto& thread_context{thread_contexts_.Get()};

  if (thread_context.active_nodes.empty()) {
    return;
  }

  auto now{GetTimestampNanoSeconds()};
  auto* node{thread_context.active_nodes.top()};
  node->end_time = now;
  node->elapsed_time_ms =
      static_cast<ProfilerElapsedTime>(node->end_time - node->start_time) /
      1000000.0f;

  thread_context.active_nodes.pop();
}

void ProfilerManager::Record() { is_recording_ = true; }

void ProfilerManager::StopRecording() { is_recording_ = false; }

void ProfilerManager::ToggleRecording() { is_recording_ = !is_recording_; }

const ProfilerData& ProfilerManager::GetData() const noexcept { return data_; }

bool ProfilerManager::IsRecording() const noexcept { return is_recording_; }

void ProfilerManager::RecordFrame() {
  if (!is_frame_recording_) {
    data_.record_context.frame_contexts.PushBack(std::nullopt);
    return;
  }

  is_frame_recording_ = false;
  auto thread_context_count{thread_contexts_.GetSize()};

  for (usize i{0}; i < thread_context_count; ++i) {
    auto thread_id{thread_contexts_.GetThreadIdFromIndex(i)};

    recording_frame_context_.thread_contexts.Set(
        thread_id, std::move(thread_contexts_.GetFromIndex(i)));
  }

  recording_frame_context_.end_time = GetTimestampNanoSeconds();
  recording_frame_context_.elapsed_time_ms =
      static_cast<ProfilerElapsedTime>(recording_frame_context_.end_time -
                                       recording_frame_context_.start_time) /
      1000000.0f;

  auto& frame_contexts{data_.record_context.frame_contexts};
  frame_contexts.PushBack(std::move(recording_frame_context_));
}
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING
