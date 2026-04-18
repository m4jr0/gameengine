// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_MATH_NUMERIC_UTILS_H_
#define COMET_COMET_MATH_NUMERIC_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/math/math_scalar.h"

namespace comet {
namespace math {
template <typename T>
constexpr T kEpsilonV{static_cast<T>(1e-6)};

constexpr f32 kEpsilonF32{kEpsilonV<f32>};
constexpr f64 kEpsilonF64{kEpsilonV<f64>};

template <typename T>
constexpr bool IsAlmostZero(T v, T eps = kEpsilonV<T>) noexcept {
  return Abs(v) <= eps;
}

template <typename T>
constexpr bool AreNearlyEqual(T a, T b, T eps = kEpsilonV<T>) noexcept {
  return Abs(a - b) <= eps;
}

template <typename T>
constexpr T AvoidZero(T v, T eps = kEpsilonV<T>) noexcept {
  return AlmostZero(v, eps) ? (v < T{0} ? -eps : eps) : v;
}
}  // namespace math
}  // namespace comet

#endif  // COMET_COMET_MATH_NUMERIC_UTILS_H_
