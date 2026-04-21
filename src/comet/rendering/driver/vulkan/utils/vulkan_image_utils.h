// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_IMAGE_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_IMAGE_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/type/vulkan_buffer_type.h"
#include "comet/rendering/driver/vulkan/type/vulkan_command_buffer_type.h"
#include "comet/rendering/driver/vulkan/type/vulkan_image_type.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_device.h"

namespace comet {
namespace rendering {
namespace vk {
void GenerateImage(Image& image, const Device& device, u32 width, u32 height,
                   u32 mip_levels, u32 array_layers,
                   VkSampleCountFlagBits num_samples, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage_flags,
                   VkMemoryPropertyFlags properties,
                   const schar* debug_label = nullptr);
void DestroyImage(Image& image);

VkImageView GenerateImageView(
    VkDevice device_handle, VkImage image_handle, VkFormat format,
    VkImageAspectFlags aspect_flags, u32 mip_levels, u32 base_array_layer = 0,
    u32 layer_count = 1, VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D);

bool IsImageInitialized(const Image& image) noexcept;

bool HasDepthComponent(VkFormat format);
bool HasStencilComponent(VkFormat format);

void CopyBufferToImage(VkCommandBuffer command_buffer, const Buffer& buffer,
                       const Image& image, u32 width, u32 height);
void CopyBufferToImage(const CommandData& command_data, const Buffer& buffer,
                       const Image& image, u32 width, u32 height);

void TransitionImageLayout(
    const Context& context, VkImage image_handle, VkFormat format,
    VkImageLayout old_layout, VkImageLayout new_layout, u32 mip_levels,
    u32 layer_count, u32 src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
    u32 dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_IMAGE_UTILS_H_
