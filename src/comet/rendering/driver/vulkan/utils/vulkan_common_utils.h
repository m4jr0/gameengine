// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_COMMON_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_COMMON_UTILS_H_

#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
VkShaderStageFlags ResolveStageFlags(ShaderStageFlags flags);
bool IsGraphicsStage(VkShaderStageFlags stage_flags);
bool IsComputeStage(VkShaderStageFlags stage_flags);
bool IsBindPoint(VkPipelineBindPoint bind_point,
                 VkShaderStageFlags stage_flags);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_COMMON_UTILS_H_
