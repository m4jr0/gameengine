// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_FRAME_H_
#define COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_FRAME_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"

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
  VkSemaphore render_semaphore_handle{VK_NULL_HANDLE};
};

struct FrameData {
  VkCommandPool command_pool_handle{VK_NULL_HANDLE};
  VkCommandBuffer command_buffer_handle{VK_NULL_HANDLE};
  VkSemaphore present_semaphore_handle{VK_NULL_HANDLE};
  VkFence render_fence_handle{VK_NULL_HANDLE};

  VkCommandBuffer upload_command_buffer_handle{VK_NULL_HANDLE};
  VkFence upload_fence_handle{VK_NULL_HANDLE};
  u64 upload_timeline_wait_value{0};
  bool requires_upload_ownership_acquire{false};
  bool has_upload_submission{false};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_FRAME_H_
