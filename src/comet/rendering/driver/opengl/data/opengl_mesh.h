// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MESH_H_

#include "comet_precompile.h"

#include "comet/math/vector.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace gl {
using MeshId = u64;
constexpr auto kInvalidMeshId{static_cast<MeshId>(-1)};

struct Vertex {
  math::Vec3 position{};
  math::Vec3 normal{};
  math::Vec4 color{kColorWhite, 1.0f};
  math::Vec2 uv{};
};

using Index = u32;

using VertexArrayObjectHandle = u32;
constexpr auto kInvalidVertexArrayObjectHandle{0};

using VertexBufferObjectHandle = u32;
constexpr auto kInvalidVertexBufferObjectHandle{0};

using ElementBufferObjectHandle = u32;
constexpr auto kInvalidElementBufferObjectHandle{0};

struct Mesh {
  MeshId id{kInvalidMeshId};
  VertexArrayObjectHandle vao_handle{kInvalidVertexArrayObjectHandle};
  VertexBufferObjectHandle vbo_handle{kInvalidVertexBufferObjectHandle};
  ElementBufferObjectHandle ebo_handle{kInvalidElementBufferObjectHandle};
  std::vector<Vertex> vertices{};
  std::vector<Index> indices{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MESH_H_
