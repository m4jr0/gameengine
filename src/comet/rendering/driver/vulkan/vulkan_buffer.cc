// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_buffer.h"

#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/driver/vulkan/vulkan_initializers.h"

namespace comet {
namespace rendering {
namespace vk {
void AllocatedBuffer::Destroy() {
  if (mapped_memory != nullptr) {
    Unmap();
  }

  COMET_ASSERT(allocator != VK_NULL_HANDLE, "Allocator is null!");
  COMET_ASSERT(buffer != VK_NULL_HANDLE, "Buffer is null!");
  COMET_ASSERT(allocation != VK_NULL_HANDLE, "Allocation is null!");
  vmaDestroyBuffer(allocator, buffer, allocation);
  buffer = VK_NULL_HANDLE;
  allocation = VK_NULL_HANDLE;
}

void AllocatedBuffer::Map() {
  COMET_ASSERT(allocator != VK_NULL_HANDLE, "Allocator is null!");
  COMET_ASSERT(mapped_memory == nullptr, "Buffer is already mapped!");
  COMET_CHECK_VK(vmaMapMemory(allocator, allocation, &mapped_memory),
                 "Unable to map memory for staging buffer!");
}

void AllocatedBuffer::CopyTo(void const* data, uindex length, sptrdiff offset) {
  COMET_ASSERT(mapped_memory != nullptr, "Buffer is not mapped!");
  COMET_ASSERT(offset >= 0, "Invalid offset provided");

  void* dest;

  if (offset == 0) {
    dest = mapped_memory;
  } else {
    dest = reinterpret_cast<void*>(reinterpret_cast<sptrdiff>(mapped_memory) +
                                   offset);
  }

  std::memcpy(dest, data, length);
}

void AllocatedBuffer::Unmap() {
  COMET_ASSERT(allocator != VK_NULL_HANDLE, "Allocator is null!");
  COMET_ASSERT(mapped_memory != nullptr, "Buffer is not mapped!");
  vmaUnmapMemory(allocator, allocation);
  mapped_memory = nullptr;
}

void AllocatedBuffer::SetDescriptorInfo(VkDeviceSize size,
                                        VkDeviceSize offset) {
  descriptor_info.range = size;
  descriptor_info.offset = offset;
}

bool AllocatedBuffer::IsInitialized() const noexcept {
  return allocator != VK_NULL_HANDLE && buffer != VK_NULL_HANDLE &&
         allocation != VK_NULL_HANDLE;
}

void CreateBuffer(AllocatedBuffer& buffer, VkDeviceSize size,
                  VmaAllocator allocator, VkBufferUsageFlags usage,
                  VmaMemoryUsage vma_memory_usage,
                  VkMemoryPropertyFlags memory_property_flags,
                  VmaAllocationCreateFlags vma_flags,
                  VkSharingMode sharing_mode) {
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

  COMET_CHECK_VK(vmaCreateBuffer(allocator, &buffer_info, &alloc_info,
                                 &buffer.buffer, &buffer.allocation, nullptr),
                 "Failed to create buffer");

  buffer.allocator = allocator;
}

void AllocatedImage::Destroy() {
  COMET_ASSERT(allocator != VK_NULL_HANDLE, "Allocator is null!");
  COMET_ASSERT(image != VK_NULL_HANDLE, "Image is null!");
  COMET_ASSERT(allocation != VK_NULL_HANDLE, "Allocation is null!");
  vmaDestroyImage(allocator, image, allocation);
  image = VK_NULL_HANDLE;
  allocation = VK_NULL_HANDLE;
}

bool AllocatedImage::IsInitialized() const noexcept {
  return allocator != VK_NULL_HANDLE && image != VK_NULL_HANDLE &&
         allocation != VK_NULL_HANDLE;
}

void CreateImage(AllocatedImage& allocated_image, const VulkanDevice& device,
                 VmaAllocator allocator, u32 width, u32 height, u32 mip_levels,
                 VkSampleCountFlagBits num_samples, VkFormat format,
                 VkImageTiling tiling, VkImageUsageFlags usage_flags,
                 VkMemoryPropertyFlags properties) {
  const auto& queue_family_indices{device.GetQueueFamilyIndices()};

  const std::vector<u32> family_indices{
      queue_family_indices.transfer_family.value(),
      queue_family_indices.graphics_family.value()};

  auto sharing_mode{VK_SHARING_MODE_EXCLUSIVE};
  u32 queue_family_index_count{0};
  const u32* queue_family_indices_pointer{nullptr};

  if (queue_family_indices.IsSpecificTransferFamily()) {
    sharing_mode = VK_SHARING_MODE_CONCURRENT;
    queue_family_index_count = 2;
    queue_family_indices_pointer = family_indices.data();
  }

  auto create_info{init::GetImageCreateInfo(
      width, height, mip_levels, num_samples, format, tiling, usage_flags,
      sharing_mode, queue_family_indices_pointer, queue_family_index_count)};

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.requiredFlags = properties;

  COMET_CHECK_VK(vmaCreateImage(allocator, &create_info, &alloc_info,
                                &allocated_image.image,
                                &allocated_image.allocation, VK_NULL_HANDLE),
                 "Failed to create image!");

  allocated_image.allocator = allocator;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet