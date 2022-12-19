// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_TEXTURE_MAP_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_TEXTURE_MAP_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/data/vulkan_texture.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
using SamplerId = uindex;
constexpr auto kInvalidSamplerId = static_cast<SamplerId>(-1);

struct Sampler {
  SamplerId id{kInvalidSamplerId};
  uindex ref_count{0};
  VkSampler handle{VK_NULL_HANDLE};
};

struct TextureMap {
  Sampler* sampler{nullptr};
  const Texture* texture{nullptr};
  rendering::TextureType type{rendering::TextureType::Unknown};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_TEXTURE_MAP_H_
