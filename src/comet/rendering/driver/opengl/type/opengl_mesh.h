// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_MESH_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/geometry/type/mesh.h"
#include "comet/math/vector.h"
#include "comet/rendering/driver/opengl/type/opengl_storage.h"

namespace comet {
namespace rendering {
namespace gl {
struct MeshProxy {
  bool is_alive{false};
  geometry::MeshHandle mesh_handle{};

  GLsizei vertex_count{0};
  GLsizei index_count{0};
  GLint vertex_offset{0};
  GLint index_offset{0};
};

using VertexSourceId = u64;
constexpr auto kInvalidVertexSourceId{0};

struct ShaderVertexSource {
  bool has_index_buffer{false};
  GlNativeStorageHandle vertex_buffer_native_handle{
      kInvalidGlNativeStorageHandle};
  GlNativeStorageHandle index_buffer_native_handle{
      kInvalidGlNativeStorageHandle};
  VertexSourceId vertex_source_id{0};
};

struct GpuDebugLineVertex {
  math::Vec4 position{.0f};
  math::Vec4 color{.0f};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_MESH_H_