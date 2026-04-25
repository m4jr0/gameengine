// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_UTILS_CAMERA_UTILS_H_
#define COMET_COMET_RENDERING_UTILS_CAMERA_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/rendering/type/common.h"

namespace comet {
namespace rendering {
math::Vec3 ComputeStableUpVector(const math::Vec3& forward);

math::Mat4 LookAt(const math::Vec3& eye, const math::Vec3& center,
                  const math::Vec3& up);

math::Mat4 GeneratePerspectiveMatrix(f32 vertical_fov, f32 ratio, f32 z_near,
                                     f32 z_far,
                                     ClipSpaceDepthRange depth_range);
math::Mat4 GenerateOrthographicMatrix(f32 left, f32 right, f32 bottom, f32 top,
                                      f32 z_near, f32 z_far,
                                      ClipSpaceDepthRange depth_range);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_UTILS_CAMERA_UTILS_H_