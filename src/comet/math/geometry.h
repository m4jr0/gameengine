// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_MATH_MATH_GEOMETRY_H_
#define COMET_COMET_MATH_MATH_GEOMETRY_H_

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/math/quaternion.h"
#include "comet/math/vector.h"

namespace comet {
namespace math {
constexpr auto kPi{3.14159265358979323846};

template <typename T>
constexpr T ConvertToRadians(T x) {
  return static_cast<T>(x * (kPi / 180));
}

Mat4 Rotate(const Mat4& model, f32 angle, const Vec3& axis);
Mat4 ToRotationMatrix(const Quat& rotation);
Mat4 Translate(const Mat4& model, const Vec3& translation);
Mat4 ToTranslateMatrix(const Vec3& translation);
Mat4 Scale(const Mat4& model, f32 scale_factor);
Mat4 Scale(const Mat4& model, const Vec3& scale_factors);
Mat4 ToScaleMatrix(const Vec3& scale);
Mat4 ToScaleMatrix(f32 scale_factor);
Vec3 ExtractTranslation(const Mat4& transform);
Vec3 ExtractScale(const Mat4& transform);
f32 ExtractUniformScale(const Mat4& transform);
Mat3 ExtractRotationMatrix(const Mat4& transform);
Quat ToQuaternion(const glm::mat3& rotation_matrix);
Quat ExtractRotation(const Mat4& transform);
Mat4 ComposeTransform(const Vec3& translation, const Quat& rotation,
                      const Vec3& scale);
Mat4 ComposeTransform(const Vec3& translation, const Quat& rotation, f32 scale);
void DecomposeTransform(const Mat4& transform, Vec3& translation_out,
                        Quat& rotation_out, Vec3& scale_out);
void DecomposeTransform(const Mat4& transform, Vec3& translation_out,
                        Quat& rotation_out, f32& scale_out);

template <typename T>
T Cos(T x) {
  return std::cos(x);
}

template <typename T>
T Sin(T x) {
  return std::sin(x);
}

template <typename T>
T Tan(T x) {
  return std::tan(x);
}

template <typename T>
T Acos(T x) {
  return std::acos(x);
}

template <typename T>
T Asin(T x) {
  return std::asin(x);
}

template <typename T>
T Atan(T x) {
  return std::atan(x);
}
}  // namespace math
}  // namespace comet

#endif  // COMET_COMET_MATH_MATH_GEOMETRY_H_
