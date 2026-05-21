// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_PROFILER_PROFILER_MANAGER_H_
#define COMET_RUNTIME_PROFILER_PROFILER_MANAGER_H_

#include "comet/core/essentials.h"

#ifdef COMET_PROFILING
#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/manager.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/runtime/profiler/profiler.h"
#include "comet/runtime/thread/thread_provider.h"

namespace comet {
namespace profiler {
class ProfilerManager : public Manager {
 public:
  static ProfilerManager& Get();

  ProfilerManager() = default;
  ProfilerManager(const ProfilerManager&) = delete;
  ProfilerManager(ProfilerManager&&) = delete;
  ProfilerManager& operator=(const ProfilerManager&) = delete;
  ProfilerManager& operator=(ProfilerManager&&) = delete;
  ~ProfilerManager() override = default;

  void StartFrame(frame::FrameCount frame_count);
  void EndFrame();

  void StartProfiling(const schar* label);
  void StopProfiling();

  void Record();
  void StopRecording();
  void ToggleRecording();

  const ProfilerRecordContext& GetRecordContext() const noexcept;
  bool IsRecording() const noexcept;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  using ThreadProfilerContexts =
      thread::FiberThreadProvider<ThreadProfilerContext>;

  void RecordFrame();
  void ResetThreadContexts();

  bool is_recording_{false};
  bool is_frame_recording_{false};

  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagDebug};

  ThreadProfilerContexts thread_contexts_{&allocator_};
  ProfilerRecordContext record_context_{&allocator_};
  FrameProfilerContext recording_frame_context_{};
};
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING

#ifdef COMET_PROFILING
#define COMET_PROFILER_START_FRAME(frame_count) \
  comet::profiler::ProfilerManager::Get().StartFrame(frame_count)
#define COMET_PROFILER_END_FRAME() \
  comet::profiler::ProfilerManager::Get().EndFrame()
#else
#define COMET_PROFILER_START_FRAME(frame_count)
#define COMET_PROFILER_END_FRAME()
#endif  // COMET_PROFILING

#endif  // COMET_RUNTIME_PROFILER_PROFILER_MANAGER_H_