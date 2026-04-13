// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MESH_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/driver/opengl/data/opengl_storage.h"

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

using VertexSourceId = u64;
constexpr auto kInvalidVertexSourceId{0};

struct ShaderVertexSource {
  bool has_index_buffer{false};
  StorageHandle vertex_buffer_handle{kInvalidStorageHandle};
  StorageHandle index_buffer_handle{kInvalidStorageHandle};
  VertexSourceId vertex_source_id{0};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MESH_H_