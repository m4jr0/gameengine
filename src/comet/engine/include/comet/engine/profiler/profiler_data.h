// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_ENGINE_PROFILER_PROFILER_DATA_H_
#define COMET_ENGINE_PROFILER_PROFILER_DATA_H_

#include "comet/core/container/map.h"
#include "comet/core/essentials.h"
#include "comet/runtime/memory/memory.h"

#ifdef COMET_PROFILING
#include "comet/render/type/common.h"
#include "comet/runtime/profiler/profiler.h"

namespace comet {
namespace profiler {
struct ProfilerData {
  f64 physics_frame_time{0};
  u32 physics_frame_rate{0};
  f64 rendering_frame_time{0};
  u32 rendering_frame_rate{0};

#ifdef COMET_DEBUG_RENDERING
  u32 rendering_draw_count{0};
#endif  // COMET_DEBUG_RENDERING

  render::DriverType rendering_driver_type{render::DriverType::Unknown};
  usize memory_use{0};
  usize entity_count{0};
  usize entity_capacity{0};
  usize pending_entity_count{0};
  schar uptime[32]{'\0'};

  Map<memory::MemoryTag, usize> tag_use{};
  const ProfilerRecordContext* record_context{nullptr};

  explicit ProfilerData(memory::Allocator* allocator = nullptr);
  ProfilerData(const ProfilerData&) = delete;
  ProfilerData(ProfilerData&& other) noexcept;
  ProfilerData& operator=(const ProfilerData&) = delete;
  ProfilerData& operator=(ProfilerData&& other) noexcept;
  ~ProfilerData() = default;
};
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING

#endif  // COMET_ENGINE_PROFILER_PROFILER_DATA_H_