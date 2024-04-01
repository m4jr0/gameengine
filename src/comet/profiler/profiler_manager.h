// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_PROFILER_PROFILER_MANAGER_H_
#define COMET_COMET_PROFILER_PROFILER_MANAGER_H_

#include "comet/core/essentials.h"

#ifdef COMET_PROFILING
#include "comet/core/manager.h"

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

  void Update() const;
};
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING

#endif  // COMET_COMET_PROFILER_PROFILER_MANAGER_H_
