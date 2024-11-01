// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "profiler_manager.h"

#include "comet/core/memory/allocation_tracking.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/rendering_manager.h"

#ifdef COMET_DEBUG
#include "comet/rendering/debugger/debugger_displayer_manager.h"
#endif  // COMET_DEBUG

#ifdef COMET_PROFILING
namespace comet {
namespace profiler {
ProfilerManager& ProfilerManager::Get() {
  static ProfilerManager singleton{};
  return singleton;
}

void ProfilerManager::Update() const {
#ifdef COMET_DEBUG
  auto& physics_manager{physics::PhysicsManager::Get()};
  auto& rendering_manager{rendering::RenderingManager::Get()};

  rendering::MiniProfilerPacket packet{};
  packet.physics_frame_time = physics_manager.GetFrameTime();
  packet.physics_frame_rate = physics_manager.GetFrameRate();
  packet.rendering_driver_type = rendering_manager.GetDriverType();
  packet.rendering_frame_time = rendering_manager.GetFrameTime();
  packet.rendering_frame_rate = rendering_manager.GetFrameRate();
  packet.rendering_draw_count = rendering_manager.GetDrawCount();
  COMET_GET_MEMORY_USE(packet.memory_use);
  COMET_GET_TAG_USE(packet.tag_use);

  rendering::DebuggerDisplayerManager::Get().Update(packet);
#endif  // COMET_DEBUG
}
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING
