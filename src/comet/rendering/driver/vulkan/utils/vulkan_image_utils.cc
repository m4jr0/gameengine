// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_image_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_container.h"
#include "comet/core/type/array.h"
#include "comet/core/type_trait.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_alloc.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
void GenerateImage(Image& image, const Device& device, u32 width, u32 height,
                   u32 mip_levels, u32 array_layers,
                   VkSampleCountFlagBits num_samples, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage_flags,
                   VkMemoryPropertyFlags properties,
                   [[maybe_unused]] const schar* debug_label) {
  COMET_ASSERT(image.allocator_handle != VK_NULL_HANDLE,
               "vulkan_image_utils::GenerateImage",
               "allocator handle is invalid");
  COMET_ASSERT(width > 0, "vulkan_image_utils::GenerateImage",
               "image width is zero");
  COMET_ASSERT(height > 0, "vulkan_image_utils::GenerateImage",
               "image height is zero");
  COMET_ASSERT(mip_levels > 0, "vulkan_image_utils::GenerateImage",
               "mip level count is zero");
  COMET_ASSERT(array_layers > 0, "vulkan_image_utils::GenerateImage",
               "array layer count is zero");
  COMET_ASSERT(format != VK_FORMAT_UNDEFINED,
               "vulkan_image_utils::GenerateImage",
               "image format is undefined");

  auto& queue_family_indices{device.GetQueueFamilyIndices()};

  auto sharing_mode{VK_SHARING_MODE_EXCLUSIVE};
  u32 queue_family_index_count{0};
  u32* queue_family_indices_pointer{nullptr};

  frame::FrameArray<u32> family_indices{};
  family_indices.Reserve(2);

  COMET_ASSERT(queue_family_indices.graphics_family.has_value(),
               "vulkan_image_utils::GenerateImage",
               "graphics queue family index is missing");
  family_indices.PushLast(queue_family_indices.graphics_family.value());

  if (IsTransferFamilyInQueueFamilyIndices(queue_family_indices)) {
    COMET_ASSERT(queue_family_indices.transfer_family.has_value(),
                 "vulkan_image_utils::GenerateImage",
                 "transfer queue family index is missing");

    if (queue_family_indices.transfer_family.value() !=
        queue_family_indices.graphics_family.value()) {
      family_indices.PushLast(queue_family_indices.transfer_family.value());
      sharing_mode = VK_SHARING_MODE_CONCURRENT;
      queue_family_index_count = 2;
      queue_family_indices_pointer = family_indices.GetData();
    }
  }

  const auto create_info{init::GenerateImageCreateInfo(
      width, height, mip_levels, array_layers, num_samples, format, tiling,
      usage_flags, sharing_mode, queue_family_indices_pointer,
      queue_family_index_count)};

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.requiredFlags = properties;

  COMET_CHECK_VK(
      vmaCreateImage(image.allocator_handle, &create_info, &alloc_info,
                     &image.handle, &image.allocation_handle, VK_NULL_HANDLE),
      "vulkan_image_utils::GenerateImage", "image creation failed");

  COMET_VK_SET_DEBUG_LABEL(image.handle,
                           debug_label != nullptr ? debug_label : "image");

  image.image_view_handle =
      VK_NULL_HANDLE;  // Setting a null view explicitly. Can be
                       // created at a later point, if needed.
}

void DestroyImage(Image& image) {
  COMET_ASSERT(image.allocator_handle != VK_NULL_HANDLE,
               "vulkan_image_utils::DestroyImage",
               "allocator handle is invalid");
  COMET_ASSERT(image.handle != VK_NULL_HANDLE,
               "vulkan_image_utils::DestroyImage", "image handle is invalid");
  COMET_ASSERT(image.allocation_handle != VK_NULL_HANDLE,
               "vulkan_image_utils::DestroyImage",
               "allocation handle is invalid");

  vmaDestroyImage(image.allocator_handle, image.handle,
                  image.allocation_handle);
  image.handle = VK_NULL_HANDLE;
  image.allocation_handle = VK_NULL_HANDLE;
}

VkImageView GenerateImageView(VkDevice device_handle, VkImage image_handle,
                              VkFormat format, VkImageAspectFlags aspect_flags,
                              u32 mip_levels, u32 base_array_layer,
                              u32 layer_count, VkImageViewType view_type) {
  COMET_ASSERT(device_handle != VK_NULL_HANDLE,
               "vulkan_image_utils::GenerateImageView",
               "device handle is invalid");
  COMET_ASSERT(image_handle != VK_NULL_HANDLE,
               "vulkan_image_utils::GenerateImageView",
               "image handle is invalid");
  COMET_ASSERT(format != VK_FORMAT_UNDEFINED,
               "vulkan_image_utils::GenerateImageView",
               "image format is undefined");
  COMET_ASSERT(mip_levels > 0, "vulkan_image_utils::GenerateImageView",
               "mip level count is zero");
  COMET_ASSERT(layer_count > 0, "vulkan_image_utils::GenerateImageView",
               "layer count is zero");

  const auto create_info{init::GenerateImageViewCreateInfo(
      image_handle, format, aspect_flags, mip_levels, base_array_layer,
      layer_count, view_type)};

  VkImageView image_view_handle{VK_NULL_HANDLE};

  COMET_CHECK_VK(
      vkCreateImageView(device_handle, &create_info,
                        MemoryCallbacks::Get().GetAllocCallbacksHandle(),
                        &image_view_handle),
      "vulkan_image_utils::GenerateImageView", "image view creation failed");

  return image_view_handle;
}

bool IsImageInitialized(const Image& image) noexcept {
  return image.allocator_handle != VK_NULL_HANDLE &&
         image.handle != VK_NULL_HANDLE &&
         image.allocation_handle != VK_NULL_HANDLE;
}

bool HasDepthComponent(VkFormat format) {
  return format == VK_FORMAT_D16_UNORM ||
         format == VK_FORMAT_X8_D24_UNORM_PACK32 ||
         format == VK_FORMAT_D32_SFLOAT ||
         format == VK_FORMAT_D16_UNORM_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT ||
         format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

bool HasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void CopyBufferToImage(VkCommandBuffer command_buffer_handle,
                       const Buffer& buffer, const Image& image, u32 width,
                       u32 height, u32 layer_count) {
  COMET_ASSERT(command_buffer_handle != VK_NULL_HANDLE,
               "vulkan_image_utils::CopyBufferToImage",
               "command buffer handle is invalid");
  COMET_ASSERT(buffer.handle != VK_NULL_HANDLE,
               "vulkan_image_utils::CopyBufferToImage",
               "buffer handle is invalid");
  COMET_ASSERT(image.handle != VK_NULL_HANDLE,
               "vulkan_image_utils::CopyBufferToImage",
               "image handle is invalid");
  COMET_ASSERT(width > 0, "vulkan_image_utils::CopyBufferToImage",
               "width is zero");
  COMET_ASSERT(height > 0, "vulkan_image_utils::CopyBufferToImage",
               "height is zero");
  COMET_ASSERT(layer_count > 0, "vulkan_image_utils::CopyBufferToImage",
               "layer count is zero");

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = layer_count;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(command_buffer_handle, buffer.handle, image.handle,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void CopyBufferToImage(const CommandData& command_data, const Buffer& buffer,
                       const Image& image, u32 width, u32 height,
                       u32 layer_count) {
  CopyBufferToImage(command_data.command_buffer_handle, buffer, image, width,
                    height, layer_count);
}

void CmdTransitionImageLayoutGraphics(VkCommandBuffer command_buffer_handle,
                                      VkImage image_handle, VkFormat format,
                                      VkImageLayout old_layout,
                                      VkImageLayout new_layout, u32 mip_levels,
                                      u32 layer_count,
                                      u32 src_queue_family_index,
                                      u32 dst_queue_family_index) {
  COMET_ASSERT(command_buffer_handle != VK_NULL_HANDLE,
               "vulkan_image_utils::CmdTransitionImageLayoutGraphics",
               "command buffer handle is invalid");
  COMET_ASSERT(image_handle != VK_NULL_HANDLE,
               "vulkan_image_utils::CmdTransitionImageLayoutGraphics",
               "image handle is invalid");
  COMET_ASSERT(format != VK_FORMAT_UNDEFINED,
               "vulkan_image_utils::CmdTransitionImageLayoutGraphics",
               "image format is undefined");
  COMET_ASSERT(mip_levels > 0,
               "vulkan_image_utils::CmdTransitionImageLayoutGraphics",
               "mip level count is zero");
  COMET_ASSERT(layer_count > 0,
               "vulkan_image_utils::CmdTransitionImageLayoutGraphics",
               "layer count is zero");

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = src_queue_family_index;
  barrier.dstQueueFamilyIndex = dst_queue_family_index;
  barrier.image = image_handle;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = mip_levels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = layer_count;

  const bool is_depth_format{HasDepthComponent(format) ||
                             HasStencilComponent(format)};
  const bool is_depth_layout{
      new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
      new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
      old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
      old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};

  if (is_depth_format || is_depth_layout) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (HasStencilComponent(format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  VkPipelineStageFlags source_stage{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
  VkPipelineStageFlags destination_stage{VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
             new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

  } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
             new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  } else if (old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    source_stage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  } else if (old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

  } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  } else {
    COMET_ASSERT(false, "vulkan_image_utils::CmdTransitionImageLayoutGraphics",
                 "unsupported graphics queue image layout transition",
                 "old_layout_value", ToUnderlying(old_layout),
                 "new_layout_value", ToUnderlying(new_layout));
    return;
  }

  vkCmdPipelineBarrier(command_buffer_handle, source_stage, destination_stage,
                       0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void GenerateMipmaps(VkCommandBuffer command_buffer_handle,
                     VkPhysicalDevice physical_device_handle,
                     VkImage image_handle, VkFormat format, u32 width,
                     u32 height, u32 mip_levels, u32 layer_count) {
  COMET_ASSERT(command_buffer_handle != VK_NULL_HANDLE,
               "vulkan_image_utils::GenerateMipmaps",
               "command buffer handle is invalid");
  COMET_ASSERT(physical_device_handle != VK_NULL_HANDLE,
               "vulkan_image_utils::GenerateMipmaps",
               "physical device handle is invalid");
  COMET_ASSERT(image_handle != VK_NULL_HANDLE,
               "vulkan_image_utils::GenerateMipmaps",
               "image handle is invalid");
  COMET_ASSERT(format != VK_FORMAT_UNDEFINED,
               "vulkan_image_utils::GenerateMipmaps",
               "image format is undefined");
  COMET_ASSERT(width > 0, "vulkan_image_utils::GenerateMipmaps",
               "width is zero");
  COMET_ASSERT(height > 0, "vulkan_image_utils::GenerateMipmaps",
               "height is zero");
  COMET_ASSERT(mip_levels > 0, "vulkan_image_utils::GenerateMipmaps",
               "mip level count is zero");
  COMET_ASSERT(layer_count > 0, "vulkan_image_utils::GenerateMipmaps",
               "layer count is zero");

  VkFormatProperties format_properties{};
  vkGetPhysicalDeviceFormatProperties(physical_device_handle, format,
                                      &format_properties);

  COMET_ASSERT(
      static_cast<bool>(format_properties.optimalTilingFeatures &
                        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT),
      "vulkan_image_utils::GenerateMipmaps",
      "image format does not support linear blitting", "format_value",
      ToUnderlying(format));

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = image_handle;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = layer_count;
  barrier.subresourceRange.levelCount = 1;

  auto mip_width{width};
  auto mip_height{height};

  for (u32 mip_level{1}; mip_level < mip_levels; ++mip_level) {
    barrier.subresourceRange.baseMipLevel = mip_level - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer_handle, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);

    VkImageBlit blit{};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {static_cast<s32>(mip_width),
                          static_cast<s32>(mip_height), 1};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = mip_level - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = layer_count;

    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {mip_width > 1 ? static_cast<s32>(mip_width / 2) : 1,
                          mip_height > 1 ? static_cast<s32>(mip_height / 2) : 1,
                          1};
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = mip_level;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = layer_count;

    vkCmdBlitImage(command_buffer_handle, image_handle,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_handle,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                   VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer_handle, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &barrier);

    if (mip_width > 1) {
      mip_width /= 2;
    }

    if (mip_height > 1) {
      mip_height /= 2;
    }
  }

  barrier.subresourceRange.baseMipLevel = mip_levels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(command_buffer_handle, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet