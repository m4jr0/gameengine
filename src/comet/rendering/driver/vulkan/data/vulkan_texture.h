// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_TEXTURE_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_TEXTURE_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/data/vulkan_image.h"
#include "comet/resource/resource.h"

namespace comet {
namespace rendering {
namespace vk {
using TextureId = resource::ResourceId;
constexpr auto kInvalidTextureId{static_cast<TextureId>(-1)};

struct Texture {
  Image image{};
  VkFormat format{VK_FORMAT_UNDEFINED};
  TextureId id{kInvalidTextureId};
  u32 width{0};
  u32 height{0};
  u32 depth{0};
  u32 mip_levels{0};
  u8 channel_count{0};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_TEXTURE_H_
