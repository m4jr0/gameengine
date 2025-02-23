// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_material_utils.h"

#include "comet/core/hash.h"
#include "comet/core/logger.h"

namespace comet {
namespace rendering {
namespace vk {
VkFilter GetFilter(rendering::TextureFilterMode filter) {
  switch (filter) {
    case rendering::TextureFilterMode::Linear:
      return VK_FILTER_LINEAR;
    case rendering::TextureFilterMode::Nearest:
      return VK_FILTER_NEAREST;
    default:
      COMET_LOG_RENDERING_ERROR("Unsupported filter mode: ",
                                rendering::GetTextureFilterModeLabel(filter),
                                "! Using linear by default.");
      return VK_FILTER_LINEAR;
  }
}

VkSamplerAddressMode GetSamplerAddressMode(
    rendering::TextureRepeatMode repeat_mode) {
  switch (repeat_mode) {
    case rendering::TextureRepeatMode::Repeat:
      return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case rendering::TextureRepeatMode::MirroredRepeat:
      return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case rendering::TextureRepeatMode::ClampToEdge:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case rendering::TextureRepeatMode::ClampToBorder:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    default:
      // Default behavior which might occur for 1D or 2D textures, so no log
      // here.
      return VK_SAMPLER_ADDRESS_MODE_REPEAT;
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet

namespace std {
comet::rendering::vk::SamplerId hash<VkSamplerCreateInfo>::operator()(
    const VkSamplerCreateInfo& sampler_info) const {
  auto computed_hash{comet::HashCombine(
      static_cast<underlying_type_t<VkBorderColor>>(sampler_info.borderColor),
      static_cast<bool>(sampler_info.unnormalizedCoordinates))};
  computed_hash = comet::HashCombine(sampler_info.maxLod, computed_hash);
  computed_hash = comet::HashCombine(sampler_info.minLod, computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<underlying_type_t<VkCompareOp>>(sampler_info.compareOp),
      computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<bool>(sampler_info.compareEnable), computed_hash);
  computed_hash = comet::HashCombine(sampler_info.maxAnisotropy, computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<bool>(sampler_info.anisotropyEnable), computed_hash);
  computed_hash = comet::HashCombine(sampler_info.mipLodBias, computed_hash);
  computed_hash =
      comet::HashCombine(static_cast<underlying_type_t<VkSamplerAddressMode>>(
                             sampler_info.addressModeW),
                         computed_hash);
  computed_hash =
      comet::HashCombine(static_cast<underlying_type_t<VkSamplerAddressMode>>(
                             sampler_info.addressModeV),
                         computed_hash);
  computed_hash =
      comet::HashCombine(static_cast<underlying_type_t<VkSamplerAddressMode>>(
                             sampler_info.addressModeU),
                         computed_hash);
  computed_hash =
      comet::HashCombine(static_cast<underlying_type_t<VkSamplerMipmapMode>>(
                             sampler_info.mipmapMode),
                         computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<underlying_type_t<VkFilter>>(sampler_info.minFilter),
      computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<underlying_type_t<VkFilter>>(sampler_info.magFilter),
      computed_hash);
  computed_hash = comet::HashCombine(static_cast<uint32_t>(sampler_info.flags),
                                     computed_hash);
  computed_hash = comet::HashCombine(
      hash<size_t>()(reinterpret_cast<size_t>(sampler_info.pNext)),
      computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<underlying_type_t<VkStructureType>>(sampler_info.sType),
      computed_hash);
  return computed_hash;
}
}  // namespace std
