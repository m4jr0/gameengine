// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_LIGHT_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_LIGHT_H_

#include "comet/core/essentials.h"
#include "comet/rendering/light/light_type.h"

namespace comet {
namespace rendering {
namespace vk {
struct LightProxy {
  LightHandle handle{};
  LightProperties props{};
  LightShadow shadow{};

  s32 gpu_shadow_index{-1};   // First shadow entry in shadow SSBO.
  u32 shadow_entry_count{0};  // Cascade count for dir, 1 for spot, 0 for none.

  bool is_dirty{false};
};

struct GpuLight {
  math::Vec4 position_type{};
  math::Vec4 direction_intensity{};
  math::Vec4 color_range{};
  math::Vec4 shadow_header{};
  math::Vec4 spot_data{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_LIGHT_H_