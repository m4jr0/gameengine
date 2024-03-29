// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_BUFFER_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_BUFFER_UTILS_H_

#include "comet_precompile.h"

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/vulkan_device.h"

namespace comet {
namespace rendering {
namespace vk {
Buffer GenerateBuffer(VmaAllocator allocator_handle, VkDeviceSize size,
                      VkBufferUsageFlags usage, VmaMemoryUsage vma_memory_usage,
                      VkMemoryPropertyFlags memory_property_flags = 0,
                      VmaAllocationCreateFlags vma_flags = 0,
                      VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE);
void DestroyBuffer(Buffer& buffer);
void MapBuffer(Buffer& buffer);
void CopyToBuffer(Buffer& buffer, void const* data, uindex length,
                  sptrdiff offset = 0);
void UnmapBuffer(Buffer& buffer);
bool IsBufferInitialized(Buffer& buffer) noexcept;
void CopyBuffer(const Device& device, VkCommandPool command_pool_handle,
                Buffer src_buffer, Buffer dst_buffer, VkDeviceSize size);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_BUFFER_UTILS_H_
