// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_TYPE_CAMERA_H_
#define COMET_COMET_RENDERING_TYPE_CAMERA_H_

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"

namespace comet {
namespace rendering {
struct RenderCameraData {
  math::Mat4 projection_matrix{1.0f};
  math::Mat4 view_matrix{1.0f};
  math::Vec3 view_position{.0f};
  f32 near_plane{.05f};
  math::Vec3 front{.0f, .0f, -1.0f};
  f32 far_plane{1000.0f};
  math::Vec3 up{.0f, 1.0f, .0f};
  f32 fov_y_radians{.0f};
  math::Vec3 right{1.0f, .0f, .0f};
  f32 aspect_ratio{1.0f};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_TYPE_CAMERA_H_