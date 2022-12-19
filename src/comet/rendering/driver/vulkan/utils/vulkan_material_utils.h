// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_MATERIAL_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_MATERIAL_UTILS_H_

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/data/vulkan_texture_map.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
VkFilter GetFilter(rendering::TextureFilterMode filter);
VkSamplerAddressMode GetSamplerAddressMode(
    rendering::TextureRepeatMode repeat_mode);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

namespace std {
template <>
struct hash<VkSamplerCreateInfo> {
  comet::rendering::vk::SamplerId operator()(
      const VkSamplerCreateInfo& sampler_info) const;
};
}  // namespace std

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_MATERIAL_UTILS_H_
