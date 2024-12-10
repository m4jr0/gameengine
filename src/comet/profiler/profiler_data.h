// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_PROFILER_PROFILER_DATA_H_
#define COMET_COMET_PROFILER_PROFILER_DATA_H_

#include "comet/core/essentials.h"

#ifdef COMET_PROFILING
#include "comet/core/memory/memory.h"
#include "comet/core/type/map.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace profiler {
struct ProfilerData {
  f32 physics_frame_time{0};
  u32 physics_frame_rate{0};
  f32 rendering_frame_time{0};
  u32 rendering_frame_rate{0};
  u32 rendering_draw_count{0};
  rendering::DriverType rendering_driver_type{rendering::DriverType::Unknown};
  usize memory_use{0};
  Map<memory::MemoryTag, usize> tag_use{};
};
}  // namespace profiler
}  // namespace comet
#endif  // COMET_PROFILING

#endif  // COMET_COMET_PROFILER_PROFILER_DATA_H_
