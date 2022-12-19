// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_MESH_H_

#include "comet_precompile.h"

#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"

namespace comet {
namespace rendering {
namespace vk {
using MeshId = u64;
constexpr auto kInvalidMeshId{static_cast<MeshId>(-1)};

struct Vertex {
  glm::vec3 position{};
  glm::vec3 normal{};
  glm::vec4 color{};
  glm::vec2 uv{};
};

using Index = u32;

struct Mesh {
  std::vector<Vertex> vertices{};
  std::vector<Index> indices{};
  Buffer vertex_buffer{};
  MeshId id{kInvalidMeshId};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_MESH_H_
