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

#include "comet/core/frame/frame_utils.h"
#include "comet/core/type/array.h"
#include "comet/core/type_trait.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_alloc.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/label/rendering_common_label.h"
#include "comet/rendering/label/rendering_light_label.h"
#include "comet/rendering/label/rendering_pipeline_label.h"
#include "comet/rendering/label/rendering_shader_label.h"
#include "comet/rendering/label/rendering_texture_label.h"
#include "comet/rendering/label/rendering_view_label.h"

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
  family_indices.PushBack(queue_family_indices.graphics_family.value());

  if (IsTransferFamilyInQueueFamilyIndices(queue_family_indices)) {
    COMET_ASSERT(queue_family_indices.transfer_family.has_value(),
                 "vulkan_image_utils::GenerateImage",
                 "transfer queue family index is missing");

    if (queue_family_indices.transfer_family.value() !=
        queue_family_indices.graphics_family.value()) {
      family_indices.PushBack(queue_family_indices.transfer_family.value());
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
                       u32 height) {
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
               "image width is zero");
  COMET_ASSERT(height > 0, "vulkan_image_utils::CopyBufferToImage",
               "image height is zero");

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(command_buffer_handle, buffer.handle, image.handle,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void CopyBufferToImage(const CommandData& command_data, const Buffer& buffer,
                       const Image& image, u32 width, u32 height) {
  CopyBufferToImage(command_data.command_buffer_handle, buffer, image, width,
                    height);
}

void TransitionImageLayout(const Context& context, VkImage image_handle,
                           VkFormat format, VkImageLayout old_layout,
                           VkImageLayout new_layout, u32 mip_levels,
                           u32 layer_count, u32 src_queue_family_index,
                           u32 dst_queue_family_index) {
  COMET_ASSERT(image_handle != VK_NULL_HANDLE,
               "vulkan_image_utils::TransitionImageLayout",
               "image handle is invalid");
  COMET_ASSERT(format != VK_FORMAT_UNDEFINED,
               "vulkan_image_utils::TransitionImageLayout",
               "image format is undefined");
  COMET_ASSERT(mip_levels > 0, "vulkan_image_utils::TransitionImageLayout",
               "mip level count is zero");
  COMET_ASSERT(layer_count > 0, "vulkan_image_utils::TransitionImageLayout",
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

  VkPipelineStageFlags source_stage{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
  VkPipelineStageFlags destination_stage{VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
  VkCommandPool command_pool_handle{context.GetFrameData().command_pool_handle};
  VkQueue queue_handle{context.GetDevice().GetGraphicsQueueHandle()};
  const auto& device{context.GetDevice()};

  bool is_depth_format{HasDepthComponent(format) ||
                       HasStencilComponent(format)};
  bool is_depth_layout{
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

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    command_pool_handle = context.GetTransferCommandPoolHandle();
    queue_handle = device.GetTransferQueueHandle();
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
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
  } else {
    COMET_ASSERT(false, "vulkan_image_utils::TransitionImageLayout",
                 "image layout transition is unsupported", "old_layout_value",
                 ToUnderlying(old_layout), "new_layout_value",
                 ToUnderlying(new_layout));
    return;
  }

  auto command_buffer_handle{
      GenerateOneTimeCommand(device, command_pool_handle)};

  vkCmdPipelineBarrier(command_buffer_handle, source_stage, destination_stage,
                       0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);

  SubmitOneTimeCommand(command_buffer_handle, command_pool_handle, device,
                       queue_handle);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet