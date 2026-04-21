// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_texture_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type_trait.h"
#include "comet/rendering/label/rendering_texture_label.h"
#include "comet/rendering/utils/rendering_texture_utils.h"

namespace comet {
namespace rendering {
namespace vk {
HashValue GenerateHash(const TextureKey& key) {
  HashValue hash{0};
  hash = HashCombine(hash, static_cast<HashValue>(key.kind));
  hash = HashCombine(
      hash, static_cast<HashValue>(key.texture_resource_id.GetValue()));
  hash = HashCombine(hash, static_cast<HashValue>(key.runtime_id));
  hash = HashCombine(hash, static_cast<HashValue>(key.type));
  return hash;
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

VkImageLayout GetDescriptorImageLayout(const Texture* texture) {
  COMET_ASSERT(texture != nullptr,
               "vulkan_texture_utils::GetDescriptorImageLayout",
               "texture is null");

  if (IsDepthFormat(texture->format)) {
    return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  }

  return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

VkFilter GetVkFilterMode(TextureFilterMode mode) {
  switch (mode) {
    case TextureFilterMode::Nearest:
      return VK_FILTER_NEAREST;
    case TextureFilterMode::Linear:
      return VK_FILTER_LINEAR;
    default:
      COMET_ASSERT(false, "vulkan_texture_utils::GetVkFilterMode",
                   "texture filter mode is unsupported", "filter_mode",
                   GetTextureFilterModeLabel(mode), "filter_mode_value",
                   ToUnderlying(mode));
      return VK_FILTER_LINEAR;
  }
}

VkSamplerMipmapMode GetVkMipmapMode(TextureFilterMode mode) {
  switch (mode) {
    case TextureFilterMode::Nearest:
      return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case TextureFilterMode::Linear:
      return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    default:
      COMET_ASSERT(false, "vulkan_texture_utils::GetVkMipmapMode",
                   "texture filter mode is unsupported", "filter_mode",
                   GetTextureFilterModeLabel(mode), "filter_mode_value",
                   ToUnderlying(mode));
      return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  }
}

VkSamplerAddressMode GetVkAddressMode(TextureRepeatMode mode) {
  switch (mode) {
    case TextureRepeatMode::Repeat:
      return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case TextureRepeatMode::MirroredRepeat:
      return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case TextureRepeatMode::ClampToEdge:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case TextureRepeatMode::ClampToBorder:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    default:
      COMET_ASSERT(false, "vulkan_texture_utils::GetVkAddressMode",
                   "texture repeat mode is unsupported", "repeat_mode",
                   GetTextureRepeatModeLabel(mode), "repeat_mode_value",
                   ToUnderlying(mode));
      return VK_SAMPLER_ADDRESS_MODE_REPEAT;
  }
}

VkFormat GetVkFormat(TextureFormat format, TextureType type) {
  const auto is_srgb{IsSrgbTextureType(type)};

  switch (format) {
    case TextureFormat::Rgba8:
      return is_srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;

    case TextureFormat::Rgb8:
      return is_srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;

    case TextureFormat::Unknown:
    default:
      COMET_ASSERT(false, "vulkan_texture_utils::GetVkFormat",
                   "texture format is unsupported", "format",
                   GetTextureFormatLabel(format), "format_value",
                   ToUnderlying(format));
      return VK_FORMAT_UNDEFINED;
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
