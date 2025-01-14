// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "vulkan_common_utils.h"

namespace comet {
namespace rendering {
namespace vk {
VkShaderStageFlags ResolveStageFlags(ShaderStageFlags flags) {
  VkShaderStageFlags vk_flags{0};

  if (flags & kShaderStageFlagBitsCompute) {
    vk_flags |= VK_SHADER_STAGE_COMPUTE_BIT;
  }

  if (flags & kShaderStageFlagBitsVertex) {
    vk_flags |= VK_SHADER_STAGE_VERTEX_BIT;
  }

  if (flags & kShaderStageFlagBitsFragment) {
    vk_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
  }

  return vk_flags;
}

bool IsGraphicsStage(VkShaderStageFlags stage_flags) {
  return (stage_flags &
          (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) != 0;
}

bool IsComputeStage(VkShaderStageFlags stage_flags) {
  return (stage_flags & VK_SHADER_STAGE_COMPUTE_BIT) != 0;
}

bool IsBindPoint(VkPipelineBindPoint bind_point,
                 VkShaderStageFlags stage_flags) {
  return bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS &&
             IsGraphicsStage(stage_flags) ||
         bind_point == VK_PIPELINE_BIND_POINT_COMPUTE &&
             IsComputeStage(stage_flags);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet