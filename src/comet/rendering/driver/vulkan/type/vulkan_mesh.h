// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_MESH_H_

#include "comet/core/essentials.h"
#include "comet/geometry/type/mesh.h"

namespace comet {
namespace rendering {
namespace vk {
struct MeshProxy {
  bool is_alive{false};
  geometry::MeshHandle mesh_handle{};

  u32 vertex_count{0};
  u32 index_count{0};
  u32 vertex_offset{0};
  u32 index_offset{0};
};

struct GpuDebugLineVertex {
  math::Vec4 position{.0f};
  math::Vec4 color{.0f};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_MESH_H_