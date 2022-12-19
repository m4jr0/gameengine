// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_PROXY_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_PROXY_H_

#include "glm/glm.hpp"

#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/data/vulkan_mesh.h"

namespace comet {
namespace rendering {
namespace vk {
struct RenderProxy {
  Mesh* mesh{nullptr};
  Material* material{nullptr};
  glm::mat4 transform{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_PROXY_H_
