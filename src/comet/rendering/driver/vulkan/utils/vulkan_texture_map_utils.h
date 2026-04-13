// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_TEXTURE_MAP_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_TEXTURE_MAP_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/data/vulkan_texture_map.h"

namespace comet {
namespace rendering {
namespace vk {
VkImageLayout GetDescriptorImageLayout(const TextureMap* texture_map);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_TEXTURE_MAP_UTILS_H_
