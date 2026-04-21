// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_sampler_utils.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/hash.h"
#include "comet/core/type_trait.h"
#include "comet/rendering/label/rendering_texture_label.h"

namespace comet {
namespace rendering {
namespace vk {
SamplerKey GenerateSamplerKey(const SamplerDescr& descr) {
  SamplerKey hash{0};
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.min_filter));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.mag_filter));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.mipmap_mode));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.address_mode_u));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.address_mode_v));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.address_mode_w));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.compare_enable));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.compare_op));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.border_color));
  hash = HashCombine(hash,
                     static_cast<SamplerKey>(descr.unnormalized_coordinates));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.min_lod * 1000.0f));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.max_lod * 1000.0f));
  return hash;
}

VkFilter GetFilter(TextureFilterMode filter) {
  switch (filter) {
    case TextureFilterMode::Linear:
      return VK_FILTER_LINEAR;
    case TextureFilterMode::Nearest:
      return VK_FILTER_NEAREST;
    default:
      COMET_ASSERT(false, "vulkan_sampler_utils::GetFilter",
                   "texture filter mode is unsupported", "filter_mode",
                   GetTextureFilterModeLabel(filter), "filter_mode_value",
                   ToUnderlying(filter));
      return VK_FILTER_LINEAR;
  }
}

VkSamplerAddressMode GetSamplerAddressMode(TextureRepeatMode repeat_mode) {
  switch (repeat_mode) {
    case TextureRepeatMode::Repeat:
      return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case TextureRepeatMode::MirroredRepeat:
      return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case TextureRepeatMode::ClampToEdge:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case TextureRepeatMode::ClampToBorder:
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
comet::rendering::vk::SamplerKey hash<VkSamplerCreateInfo>::operator()(
    const VkSamplerCreateInfo& sampler_info) const {
  auto computed_hash{comet::HashCombine(
      static_cast<comet::u64>(sampler_info.borderColor),
      static_cast<bool>(sampler_info.unnormalizedCoordinates))};
  computed_hash = comet::HashCombine(sampler_info.maxLod, computed_hash);
  computed_hash = comet::HashCombine(sampler_info.minLod, computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<comet::u64>(sampler_info.compareOp), computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<bool>(sampler_info.compareEnable), computed_hash);
  computed_hash = comet::HashCombine(sampler_info.maxAnisotropy, computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<bool>(sampler_info.anisotropyEnable), computed_hash);
  computed_hash = comet::HashCombine(sampler_info.mipLodBias, computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<comet::u64>(sampler_info.addressModeW), computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<comet::u64>(sampler_info.addressModeV), computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<comet::u64>(sampler_info.addressModeU), computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<comet::u64>(sampler_info.mipmapMode), computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<comet::u64>(sampler_info.minFilter), computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<comet::u64>(sampler_info.magFilter), computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<comet::u64>(sampler_info.flags), computed_hash);
  computed_hash = comet::HashCombine(
      hash<comet::u64>()(reinterpret_cast<comet::u64>(sampler_info.pNext)),
      computed_hash);
  computed_hash = comet::HashCombine(
      static_cast<comet::u64>(sampler_info.sType), computed_hash);
  return computed_hash;
}
}  // namespace std
