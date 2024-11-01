// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_alloc.h"

#include "comet/core/memory/allocation_tracking.h"
#include "comet/core/memory/memory_general_alloc.h"

namespace comet {
namespace rendering {
namespace vk {
#ifdef COMET_RENDERING_DEBUG
namespace internal {
void* VKAPI_PTR VulkanAllocate(void*, std::size_t size, std::size_t align,
                               VkSystemAllocationScope) {
#ifdef COMET_MSVC
  auto* ptr{_aligned_malloc(size, align)};
#else
  void* ptr{nullptr};
  if (posix_memalign(&ptr, align, size) != 0) {
    ptr = nullptr;
  }
#endif  // COMET_MSVC

  if (ptr != nullptr) {
    COMET_REGISTER_PLATFORM_ALLOCATION(ptr, size,
                                       memory::kEngineMemoryTagRendering);
  }

  return ptr;
}

void* VKAPI_PTR VulkanReallocate(void*, void* ptr, std::size_t size,
                                 std::size_t align, VkSystemAllocationScope) {
  if (ptr != nullptr) {
    COMET_REGISTER_PLATFORM_DEALLOCATION(ptr);
  }

#ifdef COMET_MSVC
  auto* new_ptr{_aligned_realloc(ptr, size, align)};
#else
  void* new_ptr{nullptr};

  if (size == 0) {
    std::free(ptr);
  } else {
    new_ptr = std::aligned_malloc(size, align);

    if (new_ptr && ptr) {
      std::memcpy(new_ptr, ptr, size);
      std::free(ptr);
    }
  }
#endif  // COMET_MSVC

  if (new_ptr != nullptr) {
    COMET_REGISTER_PLATFORM_ALLOCATION(new_ptr, size,
                                       memory::kEngineMemoryTagRendering);
  }

  return new_ptr;
}

void VKAPI_PTR VulkanDeallocate(void*, void* ptr) {
  if (ptr != nullptr) {
    COMET_REGISTER_PLATFORM_DEALLOCATION(ptr);
  }

#ifdef COMET_MSVC
  _aligned_free(ptr);
#else
  std::free(ptr);
#endif  // COMET_MSVC
}

void VKAPI_PTR VulkanInternalAllocCallback(void*, std::size_t size,
                                           VkInternalAllocationType,
                                           VkSystemAllocationScope) {
  COMET_REGISTER_TAG_ALLOCATION(size,
                                memory::kEngineMemoryTagRenderingInternal);
}

void VKAPI_PTR VulkanInternalDeallocCallback(void*, std::size_t size,
                                             VkInternalAllocationType,
                                             VkSystemAllocationScope) {
  COMET_REGISTER_TAG_DEALLOCATION(size,
                                  memory::kEngineMemoryTagRenderingInternal);
}

void VKAPI_PTR VmaAllocate(VmaAllocator, std::uint32_t, VkDeviceMemory,
                           VkDeviceSize size, void*) {
  COMET_REGISTER_TAG_ALLOCATION(size, memory::kEngineMemoryTagRenderingDevice);
}

void VKAPI_PTR VmaDeallocate(VmaAllocator, std::uint32_t, VkDeviceMemory,
                             VkDeviceSize size, void*) {
  COMET_REGISTER_TAG_DEALLOCATION(size,
                                  memory::kEngineMemoryTagRenderingDevice);
}
}  // namespace internal
#endif  // COMET_RENDERING_DEBUG

MemoryCallbacks MemoryCallbacks::Get() {
  static MemoryCallbacks singleton{};
  return singleton;
}

const VkAllocationCallbacks* MemoryCallbacks::GetAllocCallbacksHandle()
    const noexcept {
#ifdef COMET_RENDERING_DEBUG
  return &allocationCallbacks;
#else
  return VK_NULL_HANDLE;
#endif  // COMET_RENDERING_DEBUG
}

const VmaDeviceMemoryCallbacks* MemoryCallbacks::GetDeviceCallbacksHandle()
    const noexcept {
#ifdef COMET_RENDERING_DEBUG
  return &memory_callbacks;
#else
  return VK_NULL_HANDLE;
#endif  // COMET_RENDERING_DEBUG;
}

MemoryCallbacks::MemoryCallbacks() {
#ifdef COMET_RENDERING_DEBUG
  allocationCallbacks.pUserData = VK_NULL_HANDLE;
  allocationCallbacks.pfnAllocation = internal::VulkanAllocate;
  allocationCallbacks.pfnReallocation = internal::VulkanReallocate;
  allocationCallbacks.pfnFree = internal::VulkanDeallocate;
  allocationCallbacks.pfnInternalAllocation =
      internal::VulkanInternalAllocCallback;
  allocationCallbacks.pfnInternalFree = internal::VulkanInternalDeallocCallback;

  memory_callbacks.pfnAllocate = internal::VmaAllocate;
  memory_callbacks.pfnFree = internal::VmaDeallocate;
#endif  // COMET_RENDERING_DEBUG
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet