// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADOW_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADOW_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/rendering/driver/vulkan/data/vulkan_image.h"

namespace comet {
namespace rendering {
namespace vk {
struct ShadowMap {
  Image depth_image{};
  VkImageView depth_image_view{};
  VkSampler sampler{};
  VkExtent2D extent{};
  math::Mat4 light_view_proj{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADOW_H_
