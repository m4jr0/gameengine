// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_COMMAND_BUFFER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_COMMAND_BUFFER_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

namespace comet {
namespace rendering {
namespace vk {
struct CommandData {
  bool is_allocated{false};
  VkDevice device_handle{VK_NULL_HANDLE};
  VkCommandBuffer command_buffer_handle{VK_NULL_HANDLE};
  VkCommandPool command_pool_handle{VK_NULL_HANDLE};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_COMMAND_BUFFER_H_
