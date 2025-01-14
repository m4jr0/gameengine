// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MESH_H_

#include "comet/core/essentials.h"
#include "comet/geometry/geometry_common.h"
#include "comet/resource/model_resource.h"

namespace comet {
namespace rendering {
namespace gl {
using VertexArrayObjectHandle = u32;
constexpr auto kInvalidVertexArrayObjectHandle{0};

using VertexBufferObjectHandle = u32;
constexpr auto kInvalidVertexBufferObjectHandle{0};

using ElementBufferObjectHandle = u32;
constexpr auto kInvalidElementBufferObjectHandle{0};

struct MeshProxy {
  geometry::MeshId id{geometry::kInvalidMeshId};
  const geometry::Mesh* mesh{nullptr};
  VertexArrayObjectHandle vao_handle{kInvalidVertexArrayObjectHandle};
  VertexBufferObjectHandle vbo_handle{kInvalidVertexBufferObjectHandle};
  ElementBufferObjectHandle ebo_handle{kInvalidElementBufferObjectHandle};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MESH_H_
