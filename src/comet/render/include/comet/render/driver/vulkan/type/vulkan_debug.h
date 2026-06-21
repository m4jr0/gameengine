// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_DEBUG_H_
#define COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_DEBUG_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/math/vector.h"

namespace comet {
namespace rendering {
namespace vk {
struct DebugGpuData {
  VkBuffer ssbo_debug_data_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_debug_data_size{0};

  VkBuffer ssbo_debug_lines_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_debug_lines_size{0};

  VkBuffer ssbo_debug_aabbs_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_debug_aabbs_size{0};

  VkBuffer ssbo_debug_camera_frustums_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_debug_camera_frustums_size{0};

  VkBuffer ssbo_debug_cascade_frustums_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_debug_cascade_frustums_size{0};

  VkBuffer ssbo_debug_light_frustums_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_debug_light_frustums_size{0};
};

struct DebugFrustumCorners {
  math::Vec4 corners[8];
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_DEBUG_H_
