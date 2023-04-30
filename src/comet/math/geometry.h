// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_MATH_MATH_GEOMETRY_H_
#define COMET_COMET_MATH_MATH_GEOMETRY_H_

#include "comet_precompile.h"

#include "comet/math/matrix.h"
#include "comet/math/vector.h"

namespace comet {
namespace math {
constexpr auto kPi{3.14159265358979323846};

template <typename T>
constexpr T ConvertToRadians(T x) {
  return x * (kPi / 180);
}

Mat4 Rotate(const Mat4& model, f32 angle, const Vec3& axis);

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
