// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_buffer_utils.h"

#include "comet/core/memory/memory.h"
#include "comet/rendering/driver/vulkan/data/vulkan_command_buffer.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
Buffer GenerateBuffer(VmaAllocator allocator_handle, VkDeviceSize size,
                      VkBufferUsageFlags usage, VmaMemoryUsage vma_memory_usage,
                      VkMemoryPropertyFlags memory_property_flags,
                      VmaAllocationCreateFlags vma_flags,
                      VkSharingMode sharing_mode,
                      [[maybe_unused]] const schar* debug_label) {
  Buffer buffer{};
  buffer.allocator_handle = allocator_handle;

  VkBufferCreateInfo buffer_info{};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = sharing_mode;
  buffer_info.flags = 0;
  buffer_info.pNext = VK_NULL_HANDLE;

  VmaAllocationCreateInfo alloc_info = {};
  alloc_info.usage = vma_memory_usage;
  alloc_info.requiredFlags = memory_property_flags;
  alloc_info.flags = vma_flags;

  COMET_CHECK_VK(
      vmaCreateBuffer(buffer.allocator_handle, &buffer_info, &alloc_info,
                      &buffer.handle, &buffer.allocation_handle, nullptr),
      "Failed to create buffer");
  COMET_VK_SET_DEBUG_LABEL(buffer.handle,
                           debug_label != nullptr ? debug_label : "buffer");
  return buffer;
}

void DestroyBuffer(Buffer& buffer) {
  if (buffer.mapped_memory != nullptr) {
    UnmapBuffer(buffer);
  }

  COMET_ASSERT(buffer.allocator_handle != VK_NULL_HANDLE,
               "Buffer allocator handle is null!");
  COMET_ASSERT(buffer.handle != VK_NULL_HANDLE, "Buffer handle is null!");
  COMET_ASSERT(buffer.allocation_handle != VK_NULL_HANDLE,
               "Buffer allocation handle is null!");
  vmaDestroyBuffer(buffer.allocator_handle, buffer.handle,
                   buffer.allocation_handle);
  buffer.handle = VK_NULL_HANDLE;
  buffer.allocation_handle = VK_NULL_HANDLE;
}

void MapBuffer(Buffer& buffer) {
  COMET_ASSERT(buffer.allocator_handle != VK_NULL_HANDLE,
               "Buffer allocator handle is null!");
  COMET_ASSERT(buffer.mapped_memory == nullptr, "Buffer is already mapped!");
  COMET_CHECK_VK(vmaMapMemory(buffer.allocator_handle, buffer.allocation_handle,
                              &buffer.mapped_memory),
                 "Unable to map memory for buffer!");
}

void CopyToBuffer(Buffer& buffer, void const* data, usize length,
                  sptrdiff offset) {
  COMET_ASSERT(buffer.mapped_memory != nullptr, "Buffer is not mapped!");
  COMET_ASSERT(offset >= 0, "Invalid offset provided");

  void* dest;

  if (offset == 0) {
    dest = buffer.mapped_memory;
  } else {
    dest = reinterpret_cast<void*>(
        reinterpret_cast<sptrdiff>(buffer.mapped_memory) + offset);
  }

  memory::CopyMemory(dest, data, length);
}

void UnmapBuffer(Buffer& buffer) {
  COMET_ASSERT(buffer.allocator_handle != VK_NULL_HANDLE,
               "Buffer allocator handle is null!");
  COMET_ASSERT(buffer.mapped_memory != nullptr, "Buffer is not mapped!");
  vmaUnmapMemory(buffer.allocator_handle, buffer.allocation_handle);
  buffer.mapped_memory = nullptr;
}

bool IsBufferInitialized(Buffer& buffer) noexcept {
  return buffer.allocator_handle != VK_NULL_HANDLE &&
         buffer.handle != VK_NULL_HANDLE &&
         buffer.allocation_handle != VK_NULL_HANDLE;
}

void CopyBuffer(const Device& device, VkCommandPool command_pool_handle,
                Buffer src_buffer, Buffer dst_buffer, VkDeviceSize size) {
  auto command_buffer_handle{
      GenerateOneTimeCommand(device, command_pool_handle)};
  VkBufferCopy copy_region{};
  copy_region.srcOffset = 0;
  copy_region.dstOffset = 0;
  copy_region.size = size;

  VkBufferMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  barrier.srcAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
  barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.buffer = dst_buffer.handle;
  barrier.offset = 0;
  barrier.size = VK_WHOLE_SIZE;

  vkCmdPipelineBarrier(command_buffer_handle,
                       VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, VK_NULL_HANDLE, 1,
                       &barrier, 0, VK_NULL_HANDLE);

  vkCmdCopyBuffer(command_buffer_handle, src_buffer.handle, dst_buffer.handle,
                  1, &copy_region);
  SubmitOneTimeCommand(command_buffer_handle, command_pool_handle, device,
                       device.GetGraphicsQueueHandle());
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet