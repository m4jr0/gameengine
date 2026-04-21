// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_SAMPLER_TYPE_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_SAMPLER_TYPE_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/rendering_handle.h"

namespace comet {
namespace rendering {
namespace vk {
using VkNativeSamplerHandle = VkSampler;
constexpr auto kInvalidVkNativeSamplerHandle{VK_NULL_HANDLE};

using SamplerKey = u64;

struct SamplerDescr {
  VkFilter min_filter{VK_FILTER_LINEAR};
  VkFilter mag_filter{VK_FILTER_LINEAR};
  VkSamplerMipmapMode mipmap_mode{VK_SAMPLER_MIPMAP_MODE_LINEAR};
  VkSamplerAddressMode address_mode_u{VK_SAMPLER_ADDRESS_MODE_REPEAT};
  VkSamplerAddressMode address_mode_v{VK_SAMPLER_ADDRESS_MODE_REPEAT};
  VkSamplerAddressMode address_mode_w{VK_SAMPLER_ADDRESS_MODE_REPEAT};
  bool compare_enable{false};
  VkCompareOp compare_op{VK_COMPARE_OP_ALWAYS};
  f32 min_lod{.0f};
  f32 max_lod{1.0f};
  VkBorderColor border_color{VK_BORDER_COLOR_INT_OPAQUE_BLACK};
  bool unnormalized_coordinates{false};
};

struct Sampler {
  SamplerKey key{0};
  SamplerHandle handle{};
  VkNativeSamplerHandle native_handle{kInvalidVkNativeSamplerHandle};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_SAMPLER_TYPE_H_