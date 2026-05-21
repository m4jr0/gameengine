// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/engine/profiler/profiler_data.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_PROFILING

#include <utility>

#include "comet/core/string/c_string.h"

namespace comet {
namespace profiler {
ProfilerData::ProfilerData(memory::Allocator* allocator) : tag_use{allocator} {}

ProfilerData::ProfilerData(ProfilerData&& other) noexcept
    : physics_frame_time{other.physics_frame_time},
      physics_frame_rate{other.physics_frame_rate},
      rendering_frame_time{other.rendering_frame_time},
      rendering_frame_rate{other.rendering_frame_rate},
#ifdef COMET_DEBUG_RENDERING
      rendering_draw_count{other.rendering_draw_count},
#endif  // COMET_DEBUG_RENDERING
      rendering_driver_type{other.rendering_driver_type},
      memory_use{other.memory_use},
      entity_count{other.entity_count},
      entity_capacity{other.entity_capacity},
      pending_entity_count{other.pending_entity_count},
      tag_use{std::move(other.tag_use)},
      record_context{other.record_context} {
  Copy(uptime, other.uptime, sizeof(uptime));
  other.physics_frame_time = .0;
  other.physics_frame_rate = 0;
  other.rendering_frame_time = .0;
  other.rendering_frame_rate = 0;
#ifdef COMET_DEBUG_RENDERING
  other.rendering_draw_count = 0;
#endif  // COMET_DEBUG_RENDERING
  other.rendering_driver_type = rendering::DriverType::Unknown;
  other.memory_use = 0;
  other.entity_count = 0;
  other.entity_capacity = 0;
  other.pending_entity_count = 0;
  other.record_context = nullptr;
  Clear(other.uptime, sizeof(other.uptime));
}

ProfilerData& ProfilerData::operator=(ProfilerData&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  physics_frame_time = other.physics_frame_time;
  physics_frame_rate = other.physics_frame_rate;
  rendering_frame_time = other.rendering_frame_time;
  rendering_frame_rate = other.rendering_frame_rate;
#ifdef COMET_DEBUG_RENDERING
  rendering_draw_count = other.rendering_draw_count;
#endif  // COMET_DEBUG_RENDERING
  rendering_driver_type = other.rendering_driver_type;
  memory_use = other.memory_use;
  entity_count = other.entity_count;
  entity_capacity = other.entity_capacity;
  pending_entity_count = other.pending_entity_count;
  tag_use = std::move(other.tag_use);
  record_context = other.record_context;

  Copy(uptime, other.uptime, sizeof(uptime));

  other.physics_frame_time = .0;
  other.physics_frame_rate = 0;
  other.rendering_frame_time = .0;
  other.rendering_frame_rate = 0;
#ifdef COMET_DEBUG_RENDERING
  other.rendering_draw_count = 0;
#endif  // COMET_DEBUG_RENDERING
  other.rendering_driver_type = rendering::DriverType::Unknown;
  other.memory_use = 0;
  other.entity_count = 0;
  other.entity_capacity = 0;
  other.pending_entity_count = 0;
  other.record_context = nullptr;
  Clear(other.uptime, sizeof(other.uptime));

  return *this;
}
}  // namespace profiler
}  // namespace comet

#endif  // COMET_PROFILING