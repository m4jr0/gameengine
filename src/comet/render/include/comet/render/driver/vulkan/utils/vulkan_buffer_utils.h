// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_BUFFER_UTILS_H_
#define COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_BUFFER_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/render/driver/vulkan/type/vulkan_buffer.h"
#include "comet/render/driver/vulkan/vulkan_device.h"

namespace comet {
namespace rendering {
namespace vk {
Buffer GenerateBuffer(VmaAllocator allocator_handle, VkDeviceSize size,
                      VkBufferUsageFlags usage, VmaMemoryUsage vma_memory_usage,
                      VkMemoryPropertyFlags memory_property_flags = 0,
                      VmaAllocationCreateFlags vma_flags = 0,
                      VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
                      const schar* debug_label = nullptr);
void DestroyBuffer(Buffer& buffer);

void MapBuffer(Buffer& buffer);
void CopyToBuffer(Buffer& buffer, void const* data, usize length,
                  sptrdiff offset = 0);
void UnmapBuffer(Buffer& buffer);

bool IsBufferInitialized(const Buffer& buffer) noexcept;

void CopyBufferImmediate(const Device& device,
                         VkCommandPool command_pool_handle, Buffer src_buffer,
                         Buffer dst_buffer, VkDeviceSize size,
                         VkQueue queue_handle,
                         BarrierDescr* barrier_descr = nullptr);

void RecreateBuffer(Buffer& buffer, VmaAllocator allocator_handle,
                    VkDeviceSize new_size, VkBufferUsageFlags usage,
                    VmaMemoryUsage vma_memory_usage,
                    VkMemoryPropertyFlags memory_property_flags,
                    VmaAllocationCreateFlags vma_flags,
                    VkSharingMode sharing_mode, const schar* debug_label);

BufferCapacityResult EnsureBufferCapacity(
    Buffer& buffer, VkCommandBuffer command_buffer_handle,
    VmaAllocator allocator_handle, VkDeviceSize required_size,
    VkBufferUsageFlags usage, VmaMemoryUsage vma_memory_usage,
    VkMemoryPropertyFlags memory_property_flags,
    VmaAllocationCreateFlags vma_flags, VkSharingMode sharing_mode,
    bool preserve_contents, const schar* debug_label);

void AddBufferMemoryBarrier(
    const Buffer& buffer, Array<VkBufferMemoryBarrier>* barriers,
    VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
    u32 src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
    u32 dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
    VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
void AddBufferMemoryBarrier(
    VkBuffer buffer_handle, Array<VkBufferMemoryBarrier>* barriers,
    VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
    u32 src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
    u32 dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
    VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
void ApplyBufferMemoryBarriers(const Array<VkBufferMemoryBarrier>& barriers,
                               VkCommandBuffer command_buffer_handle,
                               VkPipelineStageFlags src_stage_mask,
                               VkPipelineStageFlags dst_stage_mask);

class ScopedMappedBuffer {
 public:
  explicit ScopedMappedBuffer(Buffer& buffer);
  ~ScopedMappedBuffer();

 private:
  Buffer& buffer_;
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_BUFFER_UTILS_H_
