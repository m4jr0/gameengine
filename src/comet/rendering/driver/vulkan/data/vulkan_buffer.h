// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_BUFFER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_BUFFER_H_

#include "comet_precompile.h"

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

namespace comet {
namespace rendering {
namespace vk {
struct Buffer {
  VkBuffer handle{VK_NULL_HANDLE};
  VmaAllocation allocation_handle{VK_NULL_HANDLE};
  VmaAllocator allocator_handle{VK_NULL_HANDLE};
  void* mapped_memory{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_BUFFER_H_
