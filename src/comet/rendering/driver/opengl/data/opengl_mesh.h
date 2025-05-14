// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MESH_H_

#include "glad/glad.h"

#include "comet/core/essentials.h"

namespace comet {
namespace rendering {
namespace gl {
using MeshProxyHandle = usize;
constexpr auto kInvalidMeshProxyHandle{static_cast<MeshProxyHandle>(-1)};

struct MeshProxy {
  GLsizei vertex_count{0};
  GLsizei index_count{0};
  GLint vertex_offset{0};
  GLint index_offset{0};
};

struct VertexAttribute {
  GLuint index{0};
  GLint component_count{0};
  GLenum component_type{0};
  GLboolean is_normalized{GL_FALSE};
  GLsizei stride{0};
  const void* offset{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MESH_H_
