// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_ALLOC_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_ALLOC_H_

#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"

namespace comet {
namespace rendering {
namespace vk {
#ifdef COMET_DEBUG_RENDERING
namespace internal {
void* VKAPI_PTR VulkanAllocate(void*, std::size_t size, std::size_t align,
                               VkSystemAllocationScope);

void* VKAPI_PTR VulkanReallocate(void*, void* ptr, std::size_t size,
                                 std::size_t align, VkSystemAllocationScope);

void VKAPI_PTR VulkanDeallocate(void*, void* ptr);

void VKAPI_PTR VulkanInternalAllocCallback(void*,
                                           [[maybe_unused]] std::size_t size,
                                           VkInternalAllocationType,
                                           VkSystemAllocationScope);

void VKAPI_PTR VulkanInternalDeallocCallback(void*,
                                             [[maybe_unused]] std::size_t size,
                                             VkInternalAllocationType,
                                             VkSystemAllocationScope);

void VKAPI_PTR VmaAllocate(VmaAllocator, std::uint32_t, VkDeviceMemory,
                           [[maybe_unused]] VkDeviceSize size, void*);

void VKAPI_PTR VmaDeallocate(VmaAllocator, std::uint32_t, VkDeviceMemory,
                             [[maybe_unused]] VkDeviceSize size, void*);
}  // namespace internal
#endif  // COMET_DEBUG_RENDERING

struct MemoryCallbacks {
  static MemoryCallbacks Get();

  const VkAllocationCallbacks* GetAllocCallbacksHandle() const noexcept;
  const VmaDeviceMemoryCallbacks* GetDeviceCallbacksHandle() const noexcept;

 private:
  MemoryCallbacks();

  VkAllocationCallbacks allocationCallbacks = {};
  VmaDeviceMemoryCallbacks memory_callbacks = {};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_ALLOC_H_
