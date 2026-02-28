// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_IMAGE_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_IMAGE_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"

namespace comet {
namespace rendering {
namespace vk {
struct Image {
  VkImage handle{VK_NULL_HANDLE};
  VkImageView image_view_handle{VK_NULL_HANDLE};
  VmaAllocation allocation_handle{VK_NULL_HANDLE};
  VmaAllocator allocator_handle{VK_NULL_HANDLE};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_IMAGE_H_
