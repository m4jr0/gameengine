// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_common_utils.h"
////////////////////////////////////////////////////////////////////////////////

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
  return (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS &&
          IsGraphicsStage(stage_flags)) ||
         (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE &&
          IsComputeStage(stage_flags));
}

bool IsDepthFormat(VkFormat format) {
  switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
      return true;
    default:
      return false;
  }
}
VkCompareOp GetVkCompareOp(CompareOp op) {
  switch (op) {
    case CompareOp::Never:
      return VK_COMPARE_OP_NEVER;
    case CompareOp::Less:
      return VK_COMPARE_OP_LESS;
    case CompareOp::Equal:
      return VK_COMPARE_OP_EQUAL;
    case CompareOp::LessOrEqual:
      return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareOp::Greater:
      return VK_COMPARE_OP_GREATER;
    case CompareOp::NotEqual:
      return VK_COMPARE_OP_NOT_EQUAL;
    case CompareOp::GreaterOrEqual:
      return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case CompareOp::Always:
      return VK_COMPARE_OP_ALWAYS;
    default:
      COMET_ASSERT(false, "Unknown compare op provided!");
      return VK_COMPARE_OP_LESS;
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet