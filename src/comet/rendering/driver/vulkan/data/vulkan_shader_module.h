// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_MODULE_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_MODULE_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/resource/shader_module_resource.h"

namespace comet {
namespace rendering {
namespace vk {
struct ShaderModule {
  resource::ShaderModuleResourceId id{};
  ShaderModuleHandle handle{};
  usize code_size{0};
  const u32* code{nullptr};
  VkShaderModule native_handle{VK_NULL_HANDLE};
  VkShaderStageFlagBits stage{VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_MODULE_H_