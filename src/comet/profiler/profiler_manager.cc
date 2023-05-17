// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "profiler_manager.h"

#include "comet/core/memory/allocation_tracking.h"

#ifdef COMET_PROFILING
namespace comet {
namespace profiler {
ProfilerManager::ProfilerManager(const ProfilerManagerDescr& descr)
    : Manager{descr},
#ifdef COMET_DEBUG
      debugger_displayer_manager_{descr.debugger_displayer_manager},
#endif  // COMET_DEBUG
      memory_manager_{descr.memory_manager},
      physics_manager_{descr.physics_manager},
      rendering_manager_{descr.rendering_manager} {
#ifdef COMET_DEBUG
  COMET_ASSERT(debugger_displayer_manager_ != nullptr,
               "Debugger displayer manager is null!");
#endif  // COMET_DEBUG
  COMET_ASSERT(memory_manager_ != nullptr, "Memory manager is null!");
  COMET_ASSERT(physics_manager_ != nullptr, "Physics manager is null!");
  COMET_ASSERT(rendering_manager_ != nullptr, "Rendering manager is null!");
}

void ProfilerManager::Update() const {
#ifdef COMET_DEBUG
  rendering::MiniProfilerPacket packet{};
  packet.physics_frame_time = physics_manager_->GetFrameTime();
  packet.physics_frame_rate = physics_manager_->GetFrameRate();
  packet.rendering_driver_type = rendering_manager_->GetDriverType();
  packet.rendering_frame_time = rendering_manager_->GetFrameTime();
  packet.rendering_frame_rate = rendering_manager_->GetFrameRate();
  packet.rendering_draw_count = rendering_manager_->GetDrawCount();
  packet.memory_use = memory_manager_->GetAllocatedMemory();

  debugger_displayer_manager_->Update(packet);
#endif  // COMET_DEBUG
}
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING
