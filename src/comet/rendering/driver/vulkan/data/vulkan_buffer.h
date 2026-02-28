// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_BUFFER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_BUFFER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"

namespace comet {
namespace rendering {
namespace vk {
struct Buffer {
  VkDeviceSize size{0};
  VkBuffer handle{VK_NULL_HANDLE};
  VmaAllocation allocation_handle{VK_NULL_HANDLE};
  VmaAllocator allocator_handle{VK_NULL_HANDLE};
  void* mapped_memory{nullptr};
};

struct BarrierDescr {
  VkBufferMemoryBarrier barrier{};
  VkPipelineStageFlagBits src_stage_mask{VK_PIPELINE_STAGE_NONE};
  VkPipelineStageFlagBits dst_stage_mask{VK_PIPELINE_STAGE_NONE};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_BUFFER_H_
