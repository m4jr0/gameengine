// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_PROFILER_PROFILER_MANAGER_H_
#define COMET_COMET_PROFILER_PROFILER_MANAGER_H_

#include "comet/core/essentials.h"

#ifdef COMET_PROFILING
#include "comet/core/concurrency/provider/thread_provider.h"
#include "comet/core/concurrency/provider/thread_provider_manager.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/profiler/profiler.h"

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
  virtual ~ProfilerManager() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update();

  void StartFrame(frame::FrameCount frame_count);
  void EndFrame();

  void StartProfiling(const schar* label);
  void StopProfiling();

  void Record();
  void StopRecording();
  void ToggleRecording();

  const ProfilerData& GetData() const noexcept;
  bool IsRecording() const noexcept;

 private:
  using ThreadProfilerContexts =
      thread::FiberThreadProvider<ThreadProfilerContext>;

  void RecordFrame();

  ThreadProfilerContexts thread_contexts_{
      thread::ThreadProviderManager::Get()
          .AllocateFiberProvider<ThreadProfilerContext>()};

  bool is_recording_{false};
  bool is_frame_recording_{false};
  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagDebug};
  ProfilerData data_{&allocator_};
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

#endif  // COMET_COMET_PROFILER_PROFILER_MANAGER_H_
