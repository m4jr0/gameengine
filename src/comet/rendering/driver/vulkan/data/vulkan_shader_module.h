// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_MODULE_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_MODULE_H_

#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource.h"

namespace comet {
namespace rendering {
namespace vk {
using ShaderModuleId = resource::ResourceId;
constexpr auto kInvalidShaderModuleId{static_cast<ShaderModuleId>(-1)};

struct ShaderModule {
  ShaderModuleId id{kInvalidShaderModuleId};
  usize code_size{0};
  const u32* code{nullptr};
  VkShaderModule handle{VK_NULL_HANDLE};
  VkShaderStageFlagBits type{VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_MODULE_H_
