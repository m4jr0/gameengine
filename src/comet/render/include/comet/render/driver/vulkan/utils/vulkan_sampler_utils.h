// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_SAMPLER_UTILS_H_
#define COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_SAMPLER_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/type/vulkan_sampler.h"
#include "comet/rendering/type/texture.h"

namespace comet {
namespace rendering {
namespace vk {
SamplerKey GenerateSamplerKey(const SamplerDescr& descr);

VkFilter GetFilter(TextureFilterMode filter);
VkSamplerAddressMode GetSamplerAddressMode(TextureRepeatMode repeat_mode);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

namespace std {
template <>
struct hash<VkSamplerCreateInfo> {
  comet::rendering::vk::SamplerKey operator()(
      const VkSamplerCreateInfo& sampler_info) const;
};
}  // namespace std

#endif  // COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_SAMPLER_UTILS_H_
