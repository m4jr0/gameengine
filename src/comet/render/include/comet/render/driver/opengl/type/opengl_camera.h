// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_CAMERA_H_
#define COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_CAMERA_H_

#include "comet/core/essentials.h"
#include "comet/core/math/matrix.h"
#include "comet/core/math/vector.h"

namespace comet {
namespace render {
namespace gl {
struct GpuCameraData {
  math::Mat4 projection{1.0f};
  math::Mat4 view{1.0f};
  math::Vec4 view_position{0.0f};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_CAMERA_H_