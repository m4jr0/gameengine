// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "geometry.h"

#include "comet/math/math_common.h"

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

// https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation#Quaternion-derived_rotation_matrix
Mat4 ToRotationMatrix(const Quat& rotation) {
  Mat4 result{1.0f};

  auto x{rotation.x};
  auto y{rotation.y};
  auto z{rotation.z};
  auto w{rotation.w};

  auto xx{x * x};
  auto yy{y * y};
  auto zz{z * z};
  auto xy{x * y};
  auto xz{x * z};
  auto yz{y * z};
  auto wx{w * x};
  auto wy{w * y};
  auto wz{w * z};

  result[0][0] = 1.0f - 2.0f * (yy + zz);
  result[0][1] = 2.0f * (xy + wz);
  result[0][2] = 2.0f * (xz - wy);

  result[1][0] = 2.0f * (xy - wz);
  result[1][1] = 1.0f - 2.0f * (xx + zz);
  result[1][2] = 2.0f * (yz + wx);

  result[2][0] = 2.0f * (xz + wy);
  result[2][1] = 2.0f * (yz - wx);
  result[2][2] = 1.0f - 2.0f * (xx + yy);

  return result;
}

Mat4 Translate(const Mat4& model, const Vec3& translation) {
  auto translation_mat{ToTranslateMatrix(translation)};
  return model * translation_mat;
}

Mat4 ToTranslateMatrix(const Vec3& translation) {
  Mat4 result{1.0f};
  result[3][0] = translation.x;
  result[3][1] = translation.y;
  result[3][2] = translation.z;
  return result;
}

Mat4 Scale(const Mat4& model, f32 scale_factor) {
  auto scale_mat{Mat4(1.0f)};
  scale_mat[0][0] = scale_factor;
  scale_mat[1][1] = scale_factor;
  scale_mat[2][2] = scale_factor;
  return model * scale_mat;
}

Mat4 Scale(const Mat4& model, const Vec3& scale_factors) {
  auto scale_mat{ToScaleMatrix(scale_factors)};
  return model * scale_mat;
}

Mat4 ToScaleMatrix(const Vec3& scale) {
  Mat4 result{1.0f};
  result[0][0] = scale.x;
  result[1][1] = scale.y;
  result[2][2] = scale.z;
  return result;
}

Mat4 ToScaleMatrix(f32 scale_factor) {
  Mat4 result{1.0f};
  result[0][0] = scale_factor;
  result[1][1] = scale_factor;
  result[2][2] = scale_factor;
  return result;
}

Vec3 ExtractTranslation(const Mat4& transform) {
  return Vec3{transform[3][0], transform[3][1], transform[3][2]};
}

Vec3 ExtractScale(const Mat4& transform) {
  Vec3 scale{};
  scale.x =
      GetMagnitude(Vec3{transform[0][0], transform[0][1], transform[0][2]});
  scale.y =
      GetMagnitude(Vec3{transform[1][0], transform[1][1], transform[1][2]});
  scale.z =
      GetMagnitude(Vec3{transform[2][0], transform[2][1], transform[2][2]});
  return scale;
}

f32 ExtractUniformScale(const Mat4& transform) {
  Vec3 scale = ExtractScale(transform);
  return AverageComponents(scale);
}

Mat3 ExtractRotationMatrix(const Mat4& transform) {
  Vec3 scale{};
  scale.x = GetMagnitude(Vec3(transform[0]));
  scale.y = GetMagnitude(Vec3(transform[1]));
  scale.z = GetMagnitude(Vec3(transform[2]));

  Mat3 rotation{};
  rotation[0] = Vec3(transform[0]) / (scale.x != 0.0f ? scale.x : 1.0f);
  rotation[1] = Vec3(transform[1]) / (scale.y != 0.0f ? scale.y : 1.0f);
  rotation[2] = Vec3(transform[2]) / (scale.z != 0.0f ? scale.z : 1.0f);

  return rotation;
}

Quat ToQuaternion(const glm::mat3& rotation_matrix) {
  // https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
  Quat quaternion{};

  auto trace{rotation_matrix[0][0] + rotation_matrix[1][1] +
             rotation_matrix[2][2]};

  // Case: the scalar part (w) is the largest.
  if (trace > 0.0f) {
    auto s{Sqrt(trace + 1.0f) * 2.0f};
    quaternion.w = 0.25f * s;
    quaternion.x = (rotation_matrix[2][1] - rotation_matrix[1][2]) / s;
    quaternion.y = (rotation_matrix[0][2] - rotation_matrix[2][0]) / s;
    quaternion.z = (rotation_matrix[1][0] - rotation_matrix[0][1]) / s;
  }

  // Other cases: either x, y or z is dominant.
  else if (rotation_matrix[0][0] > rotation_matrix[1][1] &&
           rotation_matrix[0][0] > rotation_matrix[2][2]) {
    auto s{Sqrt(1.0f + rotation_matrix[0][0] - rotation_matrix[1][1] -
                rotation_matrix[2][2]) *
           2.0f};
    quaternion.w = (rotation_matrix[2][1] - rotation_matrix[1][2]) / s;
    quaternion.x = 0.25f * s;
    quaternion.y = (rotation_matrix[0][1] + rotation_matrix[1][0]) / s;
    quaternion.z = (rotation_matrix[0][2] + rotation_matrix[2][0]) / s;
  } else if (rotation_matrix[1][1] > rotation_matrix[2][2]) {
    auto s{Sqrt(1.0f + rotation_matrix[1][1] - rotation_matrix[0][0] -
                rotation_matrix[2][2]) *
           2.0f};
    quaternion.w = (rotation_matrix[0][2] - rotation_matrix[2][0]) / s;
    quaternion.x = (rotation_matrix[0][1] + rotation_matrix[1][0]) / s;
    quaternion.y = 0.25f * s;
    quaternion.z = (rotation_matrix[1][2] + rotation_matrix[2][1]) / s;
  } else {
    auto s{Sqrt(1.0f + rotation_matrix[2][2] - rotation_matrix[0][0] -
                rotation_matrix[1][1]) *
           2.0f};
    quaternion.w = (rotation_matrix[1][0] - rotation_matrix[0][1]) / s;
    quaternion.x = (rotation_matrix[0][2] + rotation_matrix[2][0]) / s;
    quaternion.y = (rotation_matrix[1][2] + rotation_matrix[2][1]) / s;
    quaternion.z = 0.25f * s;
  }

  return quaternion;
}

Quat ExtractRotation(const Mat4& transform) {
  return ToQuaternion(ExtractRotationMatrix(transform));
}

Mat4 ComposeTransform(const Vec3& translation, const Quat& rotation,
                      const Vec3& scale) {
  return ToTranslateMatrix(translation) * ToRotationMatrix(rotation) *
         ToScaleMatrix(scale);
}

Mat4 ComposeTransform(const Vec3& translation, const Quat& rotation,
                      f32 scale) {
  return ToTranslateMatrix(translation) * ToRotationMatrix(rotation) *
         ToScaleMatrix(scale);
}

void DecomposeTransform(const Mat4& transform, Vec3& translation_out,
                        Quat& rotation_out, Vec3& scale_out) {
  translation_out = ExtractTranslation(transform);
  scale_out = ExtractScale(transform);
  rotation_out = ExtractRotation(transform);
}

void DecomposeTransform(const Mat4& transform, Vec3& translation_out,
                        Quat& rotation_out, f32& scale_out) {
  translation_out = ExtractTranslation(transform);
  scale_out = ExtractUniformScale(transform);
  rotation_out = ExtractRotation(transform);
}
}  // namespace math
}  // namespace comet
