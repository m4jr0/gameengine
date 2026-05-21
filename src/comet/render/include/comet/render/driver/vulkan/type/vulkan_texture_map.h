// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_TEXTURE_MAP_H_
#define COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_TEXTURE_MAP_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/rendering/type/texture.h"
#include "comet/resource/texture/texture_resource.h"

namespace comet {
namespace rendering {
namespace vk {
struct TextureMap {
  SamplerHandle sampler_handle{};
  TextureHandle texture_handle{};
  resource::TextureResourceId texture_resource_id{};
  TextureType type{TextureType::Unknown};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_TEXTURE_MAP_H_