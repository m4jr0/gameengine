// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_MESH_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/geometry/geometry_common.h"
#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/model_resource.h"

namespace comet {
namespace rendering {
namespace vk {
struct MeshProxy {
  geometry::MeshId id{geometry::kInvalidMeshId};
  geometry::Mesh* mesh{nullptr};
  Buffer vertex_buffer{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_MESH_H_
