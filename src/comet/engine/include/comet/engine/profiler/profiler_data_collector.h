// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_ENGINE_PROFILER_PROFILER_DATA_COLLECTOR_H_
#define COMET_ENGINE_PROFILER_PROFILER_DATA_COLLECTOR_H_

#include "comet/core/essentials.h"

#ifdef COMET_PROFILING
#include "comet/engine/profiler/profiler_data.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"

namespace comet {
namespace profiler {
class ProfilerDataCollector {
 public:
  static ProfilerDataCollector& Get();

  ProfilerDataCollector(const ProfilerDataCollector&) = delete;
  ProfilerDataCollector(ProfilerDataCollector&&) = delete;
  ProfilerDataCollector& operator=(const ProfilerDataCollector&) = delete;
  ProfilerDataCollector& operator=(ProfilerDataCollector&&) = delete;
  ~ProfilerDataCollector() = default;

  void Update();

  const ProfilerData& GetData() const noexcept;

 private:
  ProfilerDataCollector() = default;

  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagDebug};
  ProfilerData data_{&allocator_};
};
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING

#endif  // COMET_ENGINE_PROFILER_PROFILER_DATA_COLLECTOR_H_