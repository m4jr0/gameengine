// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_texture_map_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/rendering/driver/vulkan/utils/vulkan_common_utils.h"

namespace comet {
namespace rendering {
namespace vk {
VkImageLayout GetDescriptorImageLayout(const TextureMap* texture_map) {
  COMET_ASSERT(texture_map != nullptr, "Texture map is null!");
  COMET_ASSERT(texture_map->texture != nullptr, "Texture is null!");

  if (IsDepthFormat(texture_map->texture->format)) {
    return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  }

  return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
