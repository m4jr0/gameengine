// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "rendering_utils.h"

#include "comet/math/geometry.h"

namespace comet {
namespace rendering {
math::Mat4 LookAt(const math::Vec3& eye, const math::Vec3& target,
                  const math::Vec3& world_up) {
  auto forward{target - eye};
  math::Normalize(forward);

  auto right{math::Cross(forward, world_up)};
  math::Normalize(right);

  auto up{math::Cross(right, forward)};
  math::Normalize(up);

  // Convert from world coordinates to view coordinates.
  // Following matrix is a product of a translation and rotation matrices, which
  // themselves are the transpose of their respective matrices to convert from
  // view coordinates to world coordinates.
  math::Mat4 matrix{0.0f};
  matrix[0][0] = right.x;
  matrix[1][0] = right.y;
  matrix[2][0] = right.z;
  // matrix[3][0] = 0.0f; // Useless, but just to be explicit.
  matrix[0][1] = up.x;
  matrix[1][1] = up.y;
  matrix[2][1] = up.z;
  // matrix[3][1] = 0.0f;
  matrix[0][2] = -forward.x;
  matrix[1][2] = -forward.y;
  matrix[2][2] = -forward.z;
  // matrix[3][2] = 0.0f;
  matrix[3][0] = -math::Dot(right, eye);
  matrix[3][1] = -math::Dot(up, eye);
  matrix[3][2] = math::Dot(forward, eye);
  matrix[3][3] = 1.0f;

  return matrix;
}

math::Mat4 GenerateProjectionMatrix(f32 vertical_fov, f32 ratio, f32 z_near,
                                    f32 z_far) {
  auto tan_half_fov{math::Tan(vertical_fov / 2)};
  math::Mat4 matrix{0.0f};

  // OpenGL matrix after simplicifications.
  // 2n / (r - l)
  matrix[0][0] = 1 / (ratio * tan_half_fov);
  // 2n / (t - b)
  matrix[1][1] = 1 / tan_half_fov;
  // -(f + n) / (f - n)
  matrix[2][2] = -(z_far + z_near) / (z_far - z_near);
  matrix[2][3] = -1;
  // -(2 * f * n) / (f - n)
  matrix[3][2] = -(2 * z_far * z_near) / (z_far - z_near);
  // (r + l) / (r - l)
  // matrix[2][0] = 0.0f;  // Useless, but just to be explicit.
  // (t + b) / (t - b)
  // matrix[2][1] = 0.0f;  // Useless, but just to be explicit.

  return matrix;
}
}  // namespace rendering
}  // namespace comet
