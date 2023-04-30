// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_PROFILER_PROFILER_MANAGER_H_
#define COMET_COMET_PROFILER_PROFILER_MANAGER_H_

#include "comet_precompile.h"

#ifdef COMET_PROFILING

#include "comet/core/manager.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/rendering_manager.h"

#ifdef COMET_DEBUG
#include "comet/rendering/debugger/debugger_displayer_manager.h"
#endif  // COMET_DEBUG

namespace comet {
namespace profiler {
struct ProfilerManagerDescr : ManagerDescr {
#ifdef COMET_DEBUG
  rendering::DebuggerDisplayerManager* debugger_displayer_manager{nullptr};
#endif  // COMET_DEBUG
  physics::PhysicsManager* physics_manager{nullptr};
  rendering::RenderingManager* rendering_manager{nullptr};
};

class ProfilerManager : public Manager {
 public:
  ProfilerManager() = delete;
  explicit ProfilerManager(const ProfilerManagerDescr& descr);
  ProfilerManager(const ProfilerManager&) = delete;
  ProfilerManager(ProfilerManager&&) = delete;
  ProfilerManager& operator=(const ProfilerManager&) = delete;
  ProfilerManager& operator=(ProfilerManager&&) = delete;
  virtual ~ProfilerManager() = default;

  void Update() const;

 private:
#ifdef COMET_DEBUG
  rendering::DebuggerDisplayerManager* debugger_displayer_manager_{nullptr};
#endif  // COMET_DEBUG
  physics::PhysicsManager* physics_manager_{nullptr};
  rendering::RenderingManager* rendering_manager_{nullptr};
};
}  // namespace profiler
}  // namespace comet

#endif  // COMET_PROFILING

#endif  // COMET_COMET_PROFILER_PROFILER_MANAGER_H_
