// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_IMAGE_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_IMAGE_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/entity/component/mesh_component.h"
#include "comet/rendering/driver/vulkan/vulkan_common_types.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace rendering {
namespace vk {
using VulkanTextureId = resource::ResourceId;
constexpr auto kInvalidTextureId{static_cast<VulkanTextureId>(-1)};

struct VulkanTexture {
  AllocatedImage allocation;
  VkFormat format{VK_FORMAT_UNDEFINED};
  VkImageView view{VK_NULL_HANDLE};
  VkSampler sampler{VK_NULL_HANDLE};
  VulkanTextureId id{kInvalidTextureId};
  u32 width{0};
  u32 height{0};
  u32 depth{0};
  u32 mip_levels{0};
};

u32 GetMipLevels(const resource::TextureResource* resource);
VulkanTexture GenerateVulkanTexture(const resource::TextureResource* resource);
VkFormat GetVkFormat(const resource::TextureResource* resource);

bool HasStencilComponent(VkFormat format);

VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format,
                            VkImageAspectFlags aspect_flags, u32 mip_levels);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_IMAGE_H_
