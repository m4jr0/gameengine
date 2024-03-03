// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "geometry.h"

namespace comet {
namespace math {
Mat4 Rotate(const Mat4& model, f32 angle, const Vec3& axis) {
  auto cos_angle{Cos(angle)};
  auto sin_angle{Sin(angle)};

  auto tmp_cos_x{axis.x * (1 - cos_angle)};
  auto tmp_cos_y{axis.y * (1 - cos_angle)};
  auto tmp_cos_z{axis.z * (1 - cos_angle)};

  auto tmp_sin_x{axis.x * sin_angle};
  auto tmp_sin_y{axis.y * sin_angle};
  auto tmp_sin_z{axis.z * sin_angle};

  // Compute rotation matrix from axis and angle.
  Mat4 rotation{0.0f};
  rotation[0][0] = cos_angle + axis.x * tmp_cos_x;
  rotation[0][1] = axis.y * tmp_cos_x + tmp_sin_z;
  rotation[0][2] = axis.z * tmp_cos_x - tmp_sin_y;
  rotation[1][0] = axis.x * tmp_cos_y - tmp_sin_z;
  rotation[1][1] = cos_angle + axis.y * tmp_cos_y;
  rotation[1][2] = axis.z * tmp_cos_y + tmp_sin_x;
  rotation[2][0] = axis.x * tmp_cos_z + tmp_sin_y;
  rotation[2][1] = axis.y * tmp_cos_z - tmp_sin_x;
  rotation[2][2] = cos_angle + axis.z * tmp_cos_z;

  Mat4 result{0.0f};
  result[0][0] = model[0][0] * rotation[0][0] + model[1][0] * rotation[0][1] +
                 model[2][0] * rotation[0][2];
  result[0][1] = model[0][1] * rotation[0][0] + model[1][1] * rotation[0][1] +
                 model[2][1] * rotation[0][2];
  result[0][2] = model[0][2] * rotation[0][0] + model[1][2] * rotation[0][1] +
                 model[2][2] * rotation[0][2];

  result[1][0] = model[0][0] * rotation[1][0] + model[1][0] * rotation[1][1] +
                 model[2][0] * rotation[1][2];
  result[1][1] = model[0][1] * rotation[1][0] + model[1][1] * rotation[1][1] +
                 model[2][1] * rotation[1][2];
  result[1][2] = model[0][2] * rotation[1][0] + model[1][2] * rotation[1][1] +
                 model[2][2] * rotation[1][2];

  result[2][0] = model[0][0] * rotation[2][0] + model[1][0] * rotation[2][1] +
                 model[2][0] * rotation[2][2];
  result[2][1] = model[0][1] * rotation[2][0] + model[1][1] * rotation[2][1] +
                 model[2][1] * rotation[2][2];
  result[2][2] = model[0][2] * rotation[2][0] + model[1][2] * rotation[2][1] +
                 model[2][2] * rotation[2][2];

  result[3][0] = model[3][0];
  result[3][1] = model[3][1];
  result[3][2] = model[3][2];
  result[3][3] = model[3][3];

  return result;
}
}  // namespace math
}  // namespace comet
