// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "profiler_manager.h"

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

const ProfilerData& ProfilerManager::GetData() const noexcept { return data_; }
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING
