// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/profiler/profiler_manager.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <memory>
#include <optional>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/date.h"
#include "comet/core/memory/allocation_tracking.h"
#include "comet/engine/engine_event.h"
#include "comet/entity/entity_manager.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/rendering_manager.h"
#include "comet/time/time_manager.h"
#include "comet/time/time_utils.h"

#ifdef COMET_PROFILING
namespace comet {
namespace profiler {
ProfilerManager& ProfilerManager::Get() {
  static ProfilerManager singleton{};
  return singleton;
}

void ProfilerManager::Update() {
#ifdef COMET_DEBUG
  COMET_UPDATE_MEMORY_USE_SNAPSHOT();

  auto& physics_manager{physics::PhysicsManager::Get()};
  auto& rendering_manager{rendering::RenderingManager::Get()};
  auto& entity_manager{entity::EntityManager::Get()};

  data_.physics_frame_time = physics_manager.GetFrameTime();
  data_.physics_frame_rate = physics_manager.GetFrameRate();
  data_.rendering_driver_type = rendering_manager.GetDriverType();
  data_.rendering_frame_time = rendering_manager.GetFrameTime();
  data_.rendering_frame_rate = rendering_manager.GetFrameRate();
  data_.entity_count = entity_manager.GetEntityCount();
  data_.entity_capacity = entity_manager.GetEntityCapacity();
  data_.pending_entity_count = entity_manager.GetPendingEntityCount();

#ifdef COMET_DEBUG_RENDERING
  data_.rendering_draw_count = rendering_manager.GetDrawCount();
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_TRACK_ALLOCATIONS
  const auto memory_snapshot{memory::GetLatestMemoryUseSnapshot()};
  data_.memory_use = memory_snapshot.memory_use;
  data_.tag_use.Clear();

  for (usize i{0}; i < memory_snapshot.tag_count; ++i) {
    const auto& entry{memory_snapshot.tags[i]};
    data_.tag_use.Set(entry.tag, entry.size);
  }
#else
  data_.memory_use = 0;
  data_.tag_use.Clear();
#endif  // COMET_TRACK_ALLOCATIONS

  const auto uptime{time::TimeManager::Get().GetUptime()};
  time::GetTimeString(uptime, data_.uptime, sizeof(data_.uptime));
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
  const auto thread_context_count{thread_contexts_.GetSize()};

  for (usize i{0}; i < thread_context_count; ++i) {
    auto& thread_context{thread_contexts_.GetFromIndex(i)};
    thread_context.thread_id = thread_contexts_.GetThreadIdFromIndex(i);
    thread_context.active_nodes = {};
    thread_context.root_nodes = Array<ProfilerNode*>{&allocator_};
    thread_context.nodes = Array<memory::UniquePtr<ProfilerNode>>{&allocator_};
  }

  COMET_ASSERT(thread_contexts_.IsInitialized(), "ProfilerManager::EndFrame",
               "thread contexts are not initialized");
}

void ProfilerManager::StartProfiling(const schar* label) {
  COMET_ASSERT(label != nullptr, "ProfilerManager::StartProfiling",
               "profile label is null");

  if (!is_recording_) {
    return;
  }

  auto& thread_context{thread_contexts_.Get()};
  const auto now{GetTimestampNanoSeconds()};
  auto node{std::make_unique<ProfilerNode>(&allocator_, label, now)};

  if (thread_context.active_nodes.empty()) {
    thread_context.root_nodes.PushLast(node.get());
  } else {
    thread_context.active_nodes.top()->children.PushLast(node.get());
  }

  thread_context.active_nodes.push(node.get());
  thread_context.nodes.PushLast(std::move(node));
}

void ProfilerManager::StopProfiling() {
  if (!is_recording_) {
    return;
  }

  auto& thread_context{thread_contexts_.Get()};

  if (thread_context.active_nodes.empty()) {
    return;
  }

  const auto now{GetTimestampNanoSeconds()};
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

void ProfilerManager::OnInitialize() {
  thread_contexts_.Initialize();
  const auto thread_context_count{thread_contexts_.GetSize()};

  for (thread::ThreadId i{0}; i < thread_context_count; ++i) {
    auto& thread_context{thread_contexts_.GetFromIndex(i)};
    thread_context.thread_id = thread_contexts_.GetThreadIdFromIndex(i);
  }

  RegisterEvents();

#ifdef COMET_PROFILING
#ifdef COMET_IMGUI
#endif  // COMET_IMGUI
#endif  // COMET_DEBUG
}

void ProfilerManager::OnShutdown() {
  UnregisterEvents();
  thread_contexts_.Destroy();
}

void ProfilerManager::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == ApplicationQuitEvent::kStaticType_) {
    ProfilerManager::Get().StopRecording();
  }
}

void ProfilerManager::RegisterEvents() {
  const auto event_function{
      [this](const event::Event& event) { OnEvent(event); }};

  application_quit_listener_id_ = event::EventManager::Get().Register(
      event_function, ApplicationQuitEvent::kStaticType_);
  COMET_ASSERT(application_quit_listener_id_ != event::kInvalidEventListenerId,
               "ProfilerManager::RegisterEvents",
               "application quit listener registration failed");
}

void ProfilerManager::UnregisterEvents() {
  if (application_quit_listener_id_ != event::kInvalidEventListenerId) {
    event::EventManager::Get().Unregister(application_quit_listener_id_);
    application_quit_listener_id_ = event::kInvalidEventListenerId;
  }
}

void ProfilerManager::RecordFrame() {
  if (!is_frame_recording_) {
    data_.record_context.frame_contexts.PushLast(std::nullopt);
    return;
  }

  is_frame_recording_ = false;
  const auto thread_context_count{thread_contexts_.GetSize()};

  for (usize i{0}; i < thread_context_count; ++i) {
    const auto thread_id{thread_contexts_.GetThreadIdFromIndex(i)};

    recording_frame_context_.thread_contexts.Set(
        thread_id, std::move(thread_contexts_.GetFromIndex(i)));
  }

  recording_frame_context_.end_time = GetTimestampNanoSeconds();
  recording_frame_context_.elapsed_time_ms =
      static_cast<ProfilerElapsedTime>(recording_frame_context_.end_time -
                                       recording_frame_context_.start_time) /
      1000000.0f;

  auto& frame_contexts{data_.record_context.frame_contexts};
  frame_contexts.PushLast(std::move(recording_frame_context_));
}
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING
