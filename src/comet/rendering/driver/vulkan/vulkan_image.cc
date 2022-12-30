// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_image.h"

#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/driver/vulkan/vulkan_initializers.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
u32 GetMipLevels(const resource::TextureResource* resource) {
  return static_cast<u32>(std::floor(std::log2(std::max(
             resource->descr.resolution[0], resource->descr.resolution[1])))) +
         1;
}

VulkanTexture GenerateVulkanTexture(const resource::TextureResource* resource) {
  VulkanTexture texture{};
  texture.id = resource->id;
  texture.width = resource->descr.resolution[0];
  texture.height = resource->descr.resolution[1];
  texture.depth = resource->descr.resolution[2];
  texture.mip_levels = GetMipLevels(resource);
  texture.format = GetVkFormat(resource);
  return texture;
}

VkFormat GetVkFormat(const resource::TextureResource* resource) {
  switch (resource->descr.format) {
    case (rendering::TextureFormat::Rgba8):
      return VK_FORMAT_R8G8B8A8_SRGB;
      break;
    case (rendering::TextureFormat::Rgb8):
      return VK_FORMAT_R8G8B8_SRGB;
  }

  return VK_FORMAT_UNDEFINED;
}

bool HasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format,
                            VkImageAspectFlags aspect_flags, u32 mip_levels) {
  auto create_info{
      init::GetImageViewCreateInfo(image, format, aspect_flags, 1)};
  VkImageView image_view{VK_NULL_HANDLE};

  COMET_CHECK_VK(
      vkCreateImageView(device, &create_info, VK_NULL_HANDLE, &image_view),
      "Failed to create image view");

  return image_view;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet