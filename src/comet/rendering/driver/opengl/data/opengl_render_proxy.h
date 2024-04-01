// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_RENDER_PROXY_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_RENDER_PROXY_H_

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/rendering/driver/opengl/data/opengl_material.h"
#include "comet/rendering/driver/opengl/data/opengl_mesh.h"

namespace comet {
namespace rendering {
namespace gl {
struct RenderProxy {
  MeshProxy* mesh_proxy{nullptr};
  Material* material{nullptr};
  math::Mat4 transform{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_RENDER_PROXY_H_
