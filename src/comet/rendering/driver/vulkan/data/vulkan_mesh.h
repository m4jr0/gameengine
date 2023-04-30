// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_MESH_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/math/vector.h"
#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
using MeshId = u64;
constexpr auto kInvalidMeshId{static_cast<MeshId>(-1)};

struct Vertex {
  math::Vec3 position{};
  math::Vec3 normal{};
  math::Vec4 color{kColorWhite, 1.0f};
  math::Vec2 uv{};
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
