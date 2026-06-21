// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_TEXTURE_UTILS_H_
#define COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_TEXTURE_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/render/driver/vulkan/type/vulkan_texture.h"

namespace comet {
namespace rendering {
namespace vk {
HashValue GenerateHash(const TextureKey& key);

bool IsDepthFormat(VkFormat format);
VkImageLayout GetDescriptorImageLayout(const Texture* texture);

VkFilter GetVkFilterMode(TextureFilterMode mode);
VkSamplerMipmapMode GetVkMipmapMode(TextureFilterMode mode);
VkSamplerAddressMode GetVkAddressMode(TextureRepeatMode mode);
VkFormat GetVkFormat(TextureFormat format, TextureType type);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_TEXTURE_UTILS_H_
