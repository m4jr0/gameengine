// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_BUFFER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_BUFFER_H_

#include "comet_precompile.h"

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/vulkan_device.h"

namespace comet {
namespace rendering {
namespace vk {
struct AllocatedBuffer {
  VkBuffer buffer{VK_NULL_HANDLE};
  VmaAllocation allocation{VK_NULL_HANDLE};
  VmaAllocator allocator{VK_NULL_HANDLE};
  void* mapped_memory{nullptr};
  VkDescriptorBufferInfo descriptor_info{buffer};

  void Destroy();

  void Map();
  void CopyTo(void const* data, uindex length, sptrdiff offset = 0);
  void Unmap();
  void SetDescriptorInfo(VkDeviceSize size, VkDeviceSize offset);
  bool IsInitialized() const noexcept;
};

void CreateBuffer(AllocatedBuffer& buffer, VkDeviceSize size,
                  VmaAllocator allocator, VkBufferUsageFlags usage,
                  VmaMemoryUsage vma_memory_usage,
                  VkMemoryPropertyFlags memory_property_flags = 0,
                  VmaAllocationCreateFlags vma_flags = 0,
                  VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE);

struct AllocatedImage {
  VkImage image{VK_NULL_HANDLE};
  VmaAllocation allocation{VK_NULL_HANDLE};
  VmaAllocator allocator{VK_NULL_HANDLE};

  void Destroy();

  bool IsInitialized() const noexcept;
};

void CreateImage(AllocatedImage& allocated_image, const VulkanDevice& device,
                 VmaAllocator allocator, u32 width, u32 height, u32 mip_levels,
                 VkSampleCountFlagBits num_samples, VkFormat format,
                 VkImageTiling tiling, VkImageUsageFlags usage_flags,
                 VkMemoryPropertyFlags properties);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_BUFFER_H_
