// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/engine/profiler/profiler_data_collector.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_PROFILING

#include "comet/core/time/date.h"
#include "comet/engine/profiler/profiler_data.h"
#include "comet/render/rendering/rendering_manager.h"
#include "comet/runtime/entity/entity_manager.h"
#include "comet/runtime/memory/allocation_tracking.h"
#include "comet/runtime/physics/physics_manager.h"
#include "comet/runtime/profiler/profiler_manager.h"
#include "comet/runtime/time/time_manager.h"
#include "comet/runtime/time/time_utils.h"

namespace comet {
namespace profiler {
ProfilerDataCollector& ProfilerDataCollector::Get() {
  static ProfilerDataCollector singleton{};
  return singleton;
}

void ProfilerDataCollector::Update() {
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
  data_.record_context = &ProfilerManager::Get().GetRecordContext();

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

const ProfilerData& ProfilerDataCollector::GetData() const noexcept {
  return data_;
}
}  // namespace profiler
}  // namespace comet

#endif  // COMET_PROFILING