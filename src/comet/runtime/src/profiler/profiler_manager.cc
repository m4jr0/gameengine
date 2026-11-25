// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/profiler/profiler_manager.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <memory>
#include <optional>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_PROFILING
namespace comet {
namespace profiler {
namespace {
const ProfileBackend kRuntimeProfileBackend{
    [](const schar* label) { return ProfilerManager::Get().StartProfiling(label); },
    [](ProfileToken token) { ProfilerManager::Get().StopProfiling(token); }};
}  // namespace

ProfilerManager& ProfilerManager::Get() {
  static ProfilerManager singleton{};
  return singleton;
}

void ProfilerManager::StartFrame(frame::FrameCount frame_count) {
  COMET_ASSERT(!is_frame_recording_, "ProfilerManager::StartFrame",
               "previous profiling frame has not ended");
  if (!is_recording_) {
    return;
  }

  if (record_context_.frame_contexts.IsEmpty()) {
    record_context_.start_frame_count = frame_count;
  }

  recording_frame_context_ = {&allocator_};
  recording_frame_context_.start_time = GetTimestampNanoSeconds();
  recording_frame_context_.frame_count = frame_count;
  is_frame_recording_ = true;
}

void ProfilerManager::EndFrame() {
  RecordFrame();
  ResetThreadContexts();
}

void ProfilerManager::ResetThreadContexts() {
  COMET_ASSERT(thread_contexts_.IsInitialized(),
               "ProfilerManager::ResetThreadContexts",
               "thread contexts are not initialized");
  const auto thread_context_count{thread_contexts_.GetSize()};

  for (usize i{0}; i < thread_context_count; ++i) {
    auto& thread_context{thread_contexts_.GetFromIndex(i)};
    thread_context.thread_id = thread_contexts_.GetThreadIdFromIndex(i);
    thread_context.active_nodes = {};
    thread_context.root_nodes = Array<ProfilerNode*>{&allocator_};
    thread_context.nodes = Array<memory::UniquePtr<ProfilerNode>>{&allocator_};
  }
}

ProfileToken ProfilerManager::StartProfiling(const schar* label) {
  COMET_ASSERT(label != nullptr, "ProfilerManager::StartProfiling",
               "profile label is null");

  if (!is_frame_recording_) {
    return kInvalidProfileToken;
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
  const auto token{static_cast<ProfileToken>(node->id)};
  thread_context.nodes.PushLast(std::move(node));
  return token;
}

void ProfilerManager::StopProfiling(ProfileToken token) {
  if (!is_frame_recording_) {
    return;
  }

  auto& thread_context{thread_contexts_.Get()};

  if (thread_context.active_nodes.empty() ||
      thread_context.active_nodes.top()->id != token) {
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

void ProfilerManager::Record() {
  if (is_recording_) {
    return;
  }

  // Preserve the active frame when recording is resumed mid-frame.
  if (!is_frame_recording_) {
    record_context_ = ProfilerRecordContext{&allocator_};
  }

  is_recording_ = true;
}

void ProfilerManager::StopRecording() { is_recording_ = false; }

void ProfilerManager::ToggleRecording() {
  if (is_recording_) {
    StopRecording();
  } else {
    Record();
  }
}

const ProfilerRecordContext& ProfilerManager::GetRecordContext() const noexcept {
  return record_context_;
}

bool ProfilerManager::IsRecording() const noexcept { return is_recording_; }

void ProfilerManager::OnInitialize() {
  thread_contexts_.Initialize();
  const auto thread_context_count{thread_contexts_.GetSize()};

  for (thread::ThreadId i{0}; i < thread_context_count; ++i) {
    auto& thread_context{thread_contexts_.GetFromIndex(i)};
    thread_context.thread_id = thread_contexts_.GetThreadIdFromIndex(i);
  }
  SetProfileBackend(&kRuntimeProfileBackend);
}

void ProfilerManager::OnShutdown() {
  SetProfileBackend(nullptr);
  is_recording_ = false;
  is_frame_recording_ = false;
  thread_contexts_.Destroy();
}

void ProfilerManager::RecordFrame() {
  if (!is_frame_recording_) {
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

  auto& frame_contexts{record_context_.frame_contexts};
  record_context_.end_frame_count = recording_frame_context_.frame_count;
  frame_contexts.PushLast(std::move(recording_frame_context_));
}
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING
