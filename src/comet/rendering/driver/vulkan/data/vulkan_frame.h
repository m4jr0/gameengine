// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_FRAME_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_FRAME_H_

#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"

namespace comet {
namespace rendering {
namespace vk {
using FrameIndex = u64;
constexpr auto kInvalidFrameIndex{static_cast<FrameIndex>(-1)};

using FrameInFlightIndex = u8;
constexpr auto kInvalidFrameInFlightIndex{static_cast<FrameInFlightIndex>(-1)};

using ImageIndex = u32;
constexpr auto kInvalidImageIndex{static_cast<ImageIndex>(-1)};

struct ImageData {
  ImageIndex image_index{kInvalidImageIndex};
  ImageIndex image_count{0};
};

struct FrameData {
  VkCommandPool command_pool_handle{VK_NULL_HANDLE};
  VkCommandBuffer command_buffer_handle{VK_NULL_HANDLE};
  VkSemaphore present_semaphore_handle{VK_NULL_HANDLE};
  VkSemaphore render_semaphore_handle{VK_NULL_HANDLE};
  VkFence render_fence_handle{VK_NULL_HANDLE};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_FRAME_H_
