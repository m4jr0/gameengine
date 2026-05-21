// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_TEXTURE_MAP_UTILS_H_
#define COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_TEXTURE_MAP_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/type/vulkan_texture_map.h"

namespace comet {
namespace rendering {
namespace vk {
TextureMap BuildTextureMap(SamplerHandle sampler_handle,
                           TextureHandle texture_handle,
                           resource::TextureResourceId texture_resource_id =
                               resource::TextureResourceId::Invalid(),
                           TextureType type = TextureType::Unknown);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_TEXTURE_MAP_UTILS_H_