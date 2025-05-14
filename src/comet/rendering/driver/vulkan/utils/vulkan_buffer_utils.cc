// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_buffer_utils.h"

#include "comet/core/memory/memory_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
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

  buffer.size = size;
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
  buffer.size = 0;
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

void CopyBufferImmediate(const Device& device,
                         VkCommandPool command_pool_handle, Buffer src_buffer,
                         Buffer dst_buffer, VkDeviceSize size,
                         VkQueue queue_handle, BarrierDescr* barrier_descr) {
  auto command_buffer_handle{
      GenerateOneTimeCommand(device, command_pool_handle)};
  VkBufferCopy copy_region{};
  copy_region.srcOffset = 0;
  copy_region.dstOffset = 0;
  copy_region.size = size;

  if (barrier_descr != nullptr) {
    vkCmdPipelineBarrier(command_buffer_handle, barrier_descr->src_stage_mask,
                         barrier_descr->dst_stage_mask, 0, 0, VK_NULL_HANDLE, 1,
                         &barrier_descr->barrier, 0, VK_NULL_HANDLE);
  }

  vkCmdCopyBuffer(command_buffer_handle, src_buffer.handle, dst_buffer.handle,
                  1, &copy_region);
  SubmitOneTimeCommand(command_buffer_handle, command_pool_handle, device,
                       queue_handle);
}

void ReallocateBuffer(Buffer& buffer, VmaAllocator allocator_handle,
                      VkDeviceSize new_size, VkBufferUsageFlags usage,
                      VmaMemoryUsage vma_memory_usage,
                      VkMemoryPropertyFlags memory_property_flags,
                      VmaAllocationCreateFlags vma_flags,
                      VkSharingMode sharing_mode, const schar* debug_label) {
  if (!IsBufferInitialized(buffer)) {
    buffer = GenerateBuffer(allocator_handle, new_size, usage, vma_memory_usage,
                            memory_property_flags, vma_flags, sharing_mode,
                            debug_label);
    return;
  }

  if (buffer.size >= new_size) {
    return;
  }

  auto new_buffer{GenerateBuffer(allocator_handle, new_size, usage,
                                 vma_memory_usage, memory_property_flags,
                                 vma_flags, sharing_mode, debug_label)};
  DestroyBuffer(buffer);
  buffer = new_buffer;
}

void ResizeBuffer(Buffer& buffer, const Device& device,
                  VkCommandPool command_pool_handle,
                  VmaAllocator allocator_handle, VkDeviceSize new_size,
                  VkBufferUsageFlags usage, VmaMemoryUsage vma_memory_usage,
                  VkQueue queue_handle,
                  VkMemoryPropertyFlags memory_property_flags,
                  VmaAllocationCreateFlags vma_flags,
                  VkSharingMode sharing_mode, BarrierDescr* barrier_descr,
                  const schar* debug_label) {
  if (!IsBufferInitialized(buffer)) {
    buffer = GenerateBuffer(allocator_handle, new_size, usage, vma_memory_usage,
                            memory_property_flags, vma_flags, sharing_mode,
                            debug_label);
    return;
  }

  if (buffer.size >= new_size) {
    return;
  }

  auto new_buffer{GenerateBuffer(allocator_handle, new_size, usage,
                                 vma_memory_usage, memory_property_flags,
                                 vma_flags, sharing_mode, debug_label)};

  COMET_ASSERT(
      command_pool_handle != VK_NULL_HANDLE,
      "Cannot copy current buffer's content, as no command pool was provided!");

  CopyBufferImmediate(device, command_pool_handle, buffer, new_buffer,
                      buffer.size, queue_handle, barrier_descr);

  DestroyBuffer(buffer);
  buffer = new_buffer;
}

void AddBufferMemoryBarrier(const Buffer& buffer,
                            frame::FrameArray<VkBufferMemoryBarrier>* barriers,
                            VkAccessFlags src_access_mask,
                            VkAccessFlags dst_access_mask,
                            u32 src_queue_family_index,
                            u32 dst_queue_family_index, VkDeviceSize offset,
                            VkDeviceSize size) {
  AddBufferMemoryBarrier(buffer.handle, barriers, src_access_mask,
                         dst_access_mask, src_queue_family_index,
                         dst_queue_family_index, offset, size);
}

void AddBufferMemoryBarrier(VkBuffer buffer_handle,
                            frame::FrameArray<VkBufferMemoryBarrier>* barriers,
                            VkAccessFlags src_access_mask,
                            VkAccessFlags dst_access_mask,
                            u32 src_queue_family_index,
                            u32 dst_queue_family_index, VkDeviceSize offset,
                            VkDeviceSize size) {
  COMET_ASSERT(barriers != nullptr, "Barriers is null!");

  if (buffer_handle == VK_NULL_HANDLE) {
    return;
  }

  barriers->EmplaceBack(init::GenerateBufferMemoryBarrier(
      buffer_handle, src_access_mask, dst_access_mask, src_queue_family_index,
      dst_queue_family_index, offset, size));
}

void ApplyBufferMemoryBarriers(
    frame::FrameArray<VkBufferMemoryBarrier>** barriers_ptr,
    VkCommandBuffer command_buffer_handle, VkPipelineStageFlags src_stage_mask,
    VkPipelineStageFlags dst_stage_mask) {
  auto* barriers{*barriers_ptr};

  if (barriers == nullptr) {
    return;
  }

  vkCmdPipelineBarrier(command_buffer_handle, src_stage_mask, dst_stage_mask, 0,
                       0, VK_NULL_HANDLE, static_cast<u32>(barriers->GetSize()),
                       barriers->GetData(), 0, VK_NULL_HANDLE);

  barriers = nullptr;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet