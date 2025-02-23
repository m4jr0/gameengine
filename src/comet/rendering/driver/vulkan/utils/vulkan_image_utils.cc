// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_image_utils.h"

#include "comet/core/frame/frame_utils.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_alloc.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
void GenerateImage(Image& image, const Device& device, u32 width, u32 height,
                   u32 mip_levels, VkSampleCountFlagBits num_samples,
                   VkFormat format, VkImageTiling tiling,
                   VkImageUsageFlags usage_flags,
                   VkMemoryPropertyFlags properties,
                   [[maybe_unused]] const schar* debug_label) {
  auto& queue_family_indices{device.GetQueueFamilyIndices()};

  frame::FrameArray<u32> family_indices{};
  family_indices.Reserve(2);
  family_indices.PushBack(queue_family_indices.transfer_family.value());
  family_indices.PushBack(queue_family_indices.graphics_family.value());

  auto sharing_mode{VK_SHARING_MODE_EXCLUSIVE};
  u32 queue_family_index_count{0};
  u32* queue_family_indices_pointer{nullptr};

  if (IsTransferFamilyInQueueFamilyIndices(queue_family_indices)) {
    sharing_mode = VK_SHARING_MODE_CONCURRENT;
    queue_family_index_count = 2;
    queue_family_indices_pointer = family_indices.GetData();
  }

  auto create_info{init::GenerateImageCreateInfo(
      width, height, mip_levels, num_samples, format, tiling, usage_flags,
      sharing_mode, queue_family_indices_pointer, queue_family_index_count)};

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.requiredFlags = properties;

  COMET_CHECK_VK(
      vmaCreateImage(image.allocator_handle, &create_info, &alloc_info,
                     &image.handle, &image.allocation_handle, VK_NULL_HANDLE),
      "Failed to create image!");
  COMET_VK_SET_DEBUG_LABEL(image.handle,
                           debug_label != nullptr ? debug_label : "image");

  image.image_view_handle =
      VK_NULL_HANDLE;  // Setting a null view explicitly. Can be
                       // created at a later point, if needed.
}

void DestroyImage(Image& image) {
  COMET_ASSERT(image.allocator_handle != VK_NULL_HANDLE,
               "Allocator handle is null!");
  COMET_ASSERT(image.handle != VK_NULL_HANDLE, "Image handle is null!");
  COMET_ASSERT(image.allocation_handle != VK_NULL_HANDLE,
               "Allocation handle is null!");
  vmaDestroyImage(image.allocator_handle, image.handle,
                  image.allocation_handle);
  image.handle = VK_NULL_HANDLE;
  image.allocation_handle = VK_NULL_HANDLE;
}

VkImageView GenerateImageView(VkDevice device_handle, VkImage image_handle,
                              VkFormat format, VkImageAspectFlags aspect_flags,
                              u32 mip_levels) {
  auto create_info{init::GenerateImageViewCreateInfo(image_handle, format,
                                                     aspect_flags, mip_levels)};
  VkImageView image_view_handle{VK_NULL_HANDLE};

  COMET_CHECK_VK(
      vkCreateImageView(device_handle, &create_info,
                        MemoryCallbacks::Get().GetAllocCallbacksHandle(),
                        &image_view_handle),
      "Failed to create image view");

  return image_view_handle;
}

bool IsImageInitialized(const Image& image) noexcept {
  return image.allocator_handle != VK_NULL_HANDLE &&
         image.handle != VK_NULL_HANDLE &&
         image.allocation_handle != VK_NULL_HANDLE;
}

bool HasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void CopyBufferToImage(VkCommandBuffer command_buffer_handle,
                       const Buffer& buffer, const Image& image, u32 width,
                       u32 height) {
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
                           u32 src_queue_family_index,
                           u32 dst_queue_family_index) {
  VkImageMemoryBarrier barrier{};
  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags destination_stage;
  VkCommandPool command_pool_handle;
  VkQueue queue_handle;
  auto& device{context.GetDevice()};

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
    command_pool_handle = context.GetFrameData().command_pool_handle;
    queue_handle = device.GetGraphicsQueueHandle();
  } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
             new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    command_pool_handle = context.GetFrameData().command_pool_handle;
    queue_handle = device.GetGraphicsQueueHandle();
  } else {
    COMET_ASSERT(false, "Unsupported layout transition!");
    return;
  }

  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = src_queue_family_index;
  barrier.dstQueueFamilyIndex = dst_queue_family_index;
  barrier.image = image_handle;

  if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (HasStencilComponent(format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = mip_levels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

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