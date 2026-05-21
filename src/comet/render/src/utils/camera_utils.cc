// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "render/utils/camera_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/math/geometry.h"
#include "comet/math/math_scalar.h"
#include "comet/math/numeric_utils.h"

namespace comet {
namespace rendering {
math::Vec3 ComputeStableUpVector(const math::Vec3& forward) {
  math::Vec3 dir{math::GetNormalizedCopy(forward)};
  math::Vec3 world_up{.0f, 1.0f, .0f};
  math::Vec3 fallback_up{.0f, .0f, 1.0f};

  if (math::Abs(math::Dot(dir, world_up)) > math::kParallelThreshold) {
    return fallback_up;
  }

  return world_up;
}

math::Mat4 LookAt(const math::Vec3& eye, const math::Vec3& target,
                  const math::Vec3& world_up) {
  COMET_ASSERT(!math::IsAlmostZero(math::GetSquaredMagnitude(target - eye)),
               "rendering_camera_utils::LookAt",
               "eye and target are identical");
  COMET_ASSERT(!math::IsAlmostZero(math::GetSquaredMagnitude(world_up)),
               "rendering_camera_utils::LookAt", "world up vector is zero");

  auto forward{target - eye};
  math::Normalize(forward);

  auto right{math::Cross(forward, world_up)};
  math::Normalize(right);

  auto up{math::Cross(right, forward)};
  math::Normalize(up);

  math::Mat4 matrix{.0f};
  matrix[0][0] = right.x;
  matrix[1][0] = right.y;
  matrix[2][0] = right.z;

  matrix[0][1] = up.x;
  matrix[1][1] = up.y;
  matrix[2][1] = up.z;

  matrix[0][2] = -forward.x;
  matrix[1][2] = -forward.y;
  matrix[2][2] = -forward.z;

  matrix[3][0] = -math::Dot(right, eye);
  matrix[3][1] = -math::Dot(up, eye);
  matrix[3][2] = math::Dot(forward, eye);
  matrix[3][3] = 1.0f;

  return matrix;
}

math::Mat4 GeneratePerspectiveMatrix(f32 vertical_fov, f32 ratio, f32 z_near,
                                     f32 z_far,
                                     ClipSpaceDepthRange depth_range) {
  COMET_ASSERT(ratio > .0f, "rendering_camera_utils::GeneratePerspectiveMatrix",
               "aspect ratio is invalid", "ratio", ratio);
  COMET_ASSERT(z_near > .0f,
               "rendering_camera_utils::GeneratePerspectiveMatrix",
               "near plane is invalid", "z_near", z_near);
  COMET_ASSERT(z_far > z_near,
               "rendering_camera_utils::GeneratePerspectiveMatrix",
               "far plane must be greater than near plane", "z_near", z_near,
               "z_far", z_far);

  const f32 tan_half_fov{math::Tan(vertical_fov * .5f)};
  math::Mat4 matrix{.0f};

  matrix[0][0] = 1.0f / (ratio * tan_half_fov);
  matrix[1][1] = 1.0f / tan_half_fov;
  matrix[2][3] = -1.0f;

  if (depth_range == ClipSpaceDepthRange::MinusOneToOne) {
    matrix[2][2] = -(z_far + z_near) / (z_far - z_near);
    matrix[3][2] = -(2.0f * z_far * z_near) / (z_far - z_near);
  } else {
    matrix[2][2] = -z_far / (z_far - z_near);
    matrix[3][2] = -(z_far * z_near) / (z_far - z_near);
  }

  return matrix;
}

math::Mat4 GenerateOrthographicMatrix(f32 left, f32 right, f32 bottom, f32 top,
                                      f32 z_near, f32 z_far,
                                      ClipSpaceDepthRange depth_range) {
  COMET_ASSERT(right != left,
               "rendering_camera_utils::GenerateOrthographicMatrix",
               "left and right planes are identical");
  COMET_ASSERT(top != bottom,
               "rendering_camera_utils::GenerateOrthographicMatrix",
               "top and bottom planes are identical");
  COMET_ASSERT(z_far != z_near,
               "rendering_camera_utils::GenerateOrthographicMatrix",
               "near and far planes are identical");

  math::Mat4 matrix{.0f};

  matrix[0][0] = 2.0f / (right - left);
  matrix[1][1] = 2.0f / (top - bottom);
  matrix[3][0] = -(right + left) / (right - left);
  matrix[3][1] = -(top + bottom) / (top - bottom);
  matrix[3][3] = 1.0f;

  if (depth_range == ClipSpaceDepthRange::MinusOneToOne) {
    matrix[2][2] = -2.0f / (z_far - z_near);
    matrix[3][2] = -(z_far + z_near) / (z_far - z_near);
  } else {
    matrix[2][2] = -1.0f / (z_far - z_near);
    matrix[3][2] = -z_near / (z_far - z_near);
  }

  return matrix;
}
}  // namespace rendering
}  // namespace comet