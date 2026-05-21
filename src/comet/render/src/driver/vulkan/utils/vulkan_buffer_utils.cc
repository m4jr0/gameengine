// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/utils/vulkan_buffer_utils.h"
////////////////////////////////////////////////////////////////////////////////

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
  COMET_ASSERT(allocator_handle != VK_NULL_HANDLE,
               "vulkan_buffer_utils::GenerateBuffer",
               "allocator handle is null");
  COMET_ASSERT(size > 0, "vulkan_buffer_utils::GenerateBuffer",
               "buffer size is zero");

  Buffer buffer{};
  buffer.allocator_handle = allocator_handle;

  VkBufferCreateInfo buffer_info{};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = sharing_mode;
  buffer_info.flags = 0;
  buffer_info.pNext = VK_NULL_HANDLE;

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = vma_memory_usage;
  alloc_info.requiredFlags = memory_property_flags;
  alloc_info.flags = vma_flags;

  COMET_CHECK_VK(
      vmaCreateBuffer(buffer.allocator_handle, &buffer_info, &alloc_info,
                      &buffer.handle, &buffer.allocation_handle, nullptr),
      "vulkan_buffer_utils::GenerateBuffer", "buffer creation failed");
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
               "vulkan_buffer_utils::DestroyBuffer",
               "buffer allocator handle is null");
  COMET_ASSERT(buffer.handle != VK_NULL_HANDLE,
               "vulkan_buffer_utils::DestroyBuffer", "buffer handle is null");
  COMET_ASSERT(buffer.allocation_handle != VK_NULL_HANDLE,
               "vulkan_buffer_utils::DestroyBuffer",
               "buffer allocation handle is null");
  vmaDestroyBuffer(buffer.allocator_handle, buffer.handle,
                   buffer.allocation_handle);
  buffer.handle = VK_NULL_HANDLE;
  buffer.allocation_handle = VK_NULL_HANDLE;
  buffer.size = 0;
}

void MapBuffer(Buffer& buffer) {
  COMET_ASSERT(buffer.allocator_handle != VK_NULL_HANDLE,
               "vulkan_buffer_utils::MapBuffer",
               "buffer allocator handle is null");
  COMET_ASSERT(buffer.allocation_handle != VK_NULL_HANDLE,
               "vulkan_buffer_utils::MapBuffer",
               "buffer allocation handle is null");
  COMET_ASSERT(buffer.mapped_memory == nullptr,
               "vulkan_buffer_utils::MapBuffer", "buffer is already mapped");
  COMET_CHECK_VK(vmaMapMemory(buffer.allocator_handle, buffer.allocation_handle,
                              &buffer.mapped_memory),
                 "vulkan_buffer_utils::MapBuffer",
                 "buffer memory mapping failed");
}

void CopyToBuffer(Buffer& buffer, void const* data, usize length,
                  sptrdiff offset) {
  COMET_ASSERT(buffer.mapped_memory != nullptr,
               "vulkan_buffer_utils::CopyToBuffer", "buffer is not mapped");
  COMET_ASSERT(data != nullptr, "vulkan_buffer_utils::CopyToBuffer",
               "source data is null");
  COMET_ASSERT(offset >= 0, "vulkan_buffer_utils::CopyToBuffer",
               "buffer offset is invalid", "offset", offset);
  COMET_ASSERT(static_cast<VkDeviceSize>(offset) + length <= buffer.size,
               "vulkan_buffer_utils::CopyToBuffer",
               "copy range exceeds buffer size", "offset", offset, "length",
               length, "buffer_size", buffer.size);

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
               "vulkan_buffer_utils::UnmapBuffer",
               "buffer allocator handle is null");
  COMET_ASSERT(buffer.allocation_handle != VK_NULL_HANDLE,
               "vulkan_buffer_utils::UnmapBuffer",
               "buffer allocation handle is null");
  COMET_ASSERT(buffer.mapped_memory != nullptr,
               "vulkan_buffer_utils::UnmapBuffer", "buffer is not mapped");

  vmaUnmapMemory(buffer.allocator_handle, buffer.allocation_handle);
  buffer.mapped_memory = nullptr;
}

bool IsBufferInitialized(const Buffer& buffer) noexcept {
  return buffer.allocator_handle != VK_NULL_HANDLE &&
         buffer.handle != VK_NULL_HANDLE &&
         buffer.allocation_handle != VK_NULL_HANDLE;
}

void CopyBufferImmediate(const Device& device,
                         VkCommandPool command_pool_handle, Buffer src_buffer,
                         Buffer dst_buffer, VkDeviceSize size,
                         VkQueue queue_handle, BarrierDescr* barrier_descr) {
  COMET_ASSERT(command_pool_handle != VK_NULL_HANDLE,
               "vulkan_buffer_utils::CopyBufferImmediate",
               "command pool handle is invalid");
  COMET_ASSERT(src_buffer.handle != VK_NULL_HANDLE,
               "vulkan_buffer_utils::CopyBufferImmediate",
               "source buffer handle is invalid");
  COMET_ASSERT(dst_buffer.handle != VK_NULL_HANDLE,
               "vulkan_buffer_utils::CopyBufferImmediate",
               "destination buffer handle is invalid");
  COMET_ASSERT(size > 0, "vulkan_buffer_utils::CopyBufferImmediate",
               "copy size is zero");
  COMET_ASSERT(size <= src_buffer.size,
               "vulkan_buffer_utils::CopyBufferImmediate",
               "copy size exceeds source buffer size", "copy_size", size,
               "source_buffer_size", src_buffer.size);
  COMET_ASSERT(size <= dst_buffer.size,
               "vulkan_buffer_utils::CopyBufferImmediate",
               "copy size exceeds destination buffer size", "copy_size", size,
               "destination_buffer_size", dst_buffer.size);

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

void RecreateBuffer(Buffer& buffer, VmaAllocator allocator_handle,
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

  const auto new_buffer{GenerateBuffer(allocator_handle, new_size, usage,
                                       vma_memory_usage, memory_property_flags,
                                       vma_flags, sharing_mode, debug_label)};
  DestroyBuffer(buffer);
  buffer = new_buffer;
}

BufferCapacityResult EnsureBufferCapacity(
    Buffer& buffer, VkCommandBuffer command_buffer_handle,
    VmaAllocator allocator_handle, VkDeviceSize required_size,
    VkBufferUsageFlags usage, VmaMemoryUsage vma_memory_usage,
    VkMemoryPropertyFlags memory_property_flags,
    VmaAllocationCreateFlags vma_flags, VkSharingMode sharing_mode,
    bool preserve_contents, const schar* debug_label) {
  COMET_ASSERT(command_buffer_handle != VK_NULL_HANDLE,
               "vulkan_buffer_utils::EnsureBufferCapacity",
               "command buffer handle is invalid");
  COMET_ASSERT(required_size > 0, "vulkan_buffer_utils::EnsureBufferCapacity",
               "required size is zero");

  BufferCapacityResult result{};

  if (!IsBufferInitialized(buffer)) {
    buffer = GenerateBuffer(allocator_handle, required_size, usage,
                            vma_memory_usage, memory_property_flags, vma_flags,
                            sharing_mode, debug_label);
    result.is_recreated = true;
    return result;
  }

  if (buffer.size >= required_size) {
    return result;
  }

  const auto old_size{buffer.size};
  const auto old_buffer{buffer};

  const auto new_buffer{GenerateBuffer(allocator_handle, required_size, usage,
                                       vma_memory_usage, memory_property_flags,
                                       vma_flags, sharing_mode, debug_label)};

  if (preserve_contents && old_size > 0) {
    VkBufferCopy copy{};
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = old_size;

    vkCmdCopyBuffer(command_buffer_handle, old_buffer.handle, new_buffer.handle,
                    1, &copy);
  }

  buffer = new_buffer;

  result.is_recreated = true;
  result.has_transfer_work = preserve_contents && old_size > 0;
  result.copied_size = result.has_transfer_work ? old_size : 0;
  result.old_buffer = old_buffer;
  return result;
}

void AddBufferMemoryBarrier(const Buffer& buffer,
                            Array<VkBufferMemoryBarrier>* barriers,
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
                            Array<VkBufferMemoryBarrier>* barriers,
                            VkAccessFlags src_access_mask,
                            VkAccessFlags dst_access_mask,
                            u32 src_queue_family_index,
                            u32 dst_queue_family_index, VkDeviceSize offset,
                            VkDeviceSize size) {
  COMET_ASSERT(barriers != nullptr,
               "vulkan_buffer_utils::AddBufferMemoryBarrier",
               "barriers array is null");

  if (buffer_handle == VK_NULL_HANDLE) {
    return;
  }

  barriers->EmplaceLast(init::GenerateBufferMemoryBarrier(
      buffer_handle, src_access_mask, dst_access_mask, src_queue_family_index,
      dst_queue_family_index, offset, size));
}

void ApplyBufferMemoryBarriers(const Array<VkBufferMemoryBarrier>& barriers,
                               VkCommandBuffer command_buffer_handle,
                               VkPipelineStageFlags src_stage_mask,
                               VkPipelineStageFlags dst_stage_mask) {
  COMET_ASSERT(command_buffer_handle != VK_NULL_HANDLE,
               "vulkan_buffer_utils::ApplyBufferMemoryBarriers",
               "command buffer handle is invalid");

  if (barriers.IsEmpty()) {
    return;
  }

  vkCmdPipelineBarrier(command_buffer_handle, src_stage_mask, dst_stage_mask, 0,
                       0, VK_NULL_HANDLE, static_cast<u32>(barriers.GetSize()),
                       barriers.GetData(), 0, VK_NULL_HANDLE);
}

ScopedMappedBuffer::ScopedMappedBuffer(Buffer& buffer) : buffer_{buffer} {
  MapBuffer(buffer_);
}

ScopedMappedBuffer::~ScopedMappedBuffer() { UnmapBuffer(buffer_); }
}  // namespace vk
}  // namespace rendering
}  // namespace comet