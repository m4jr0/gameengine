// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_MATH_MATH_COMMONS_H_
#define COMET_COMET_MATH_MATH_COMMONS_H_

#include <cmath>

#include "comet/core/type/primitive.h"

namespace comet {
namespace math {
template <typename T>
constexpr T CtSearchSqrt(T x, T low, T high) {
  if (low == high) {
    return low;  // low * high = x.
  }

  const T mid{(low + high + 1) / 2};

  // Result is lower than high.
  if (x / mid < mid) {
    return CtSearchSqrt<T>(x, low, mid - 1);
  }

  // Result is higher than low.
  return CtSearchSqrt(x, mid, high);
}

template <typename T>
constexpr T CtSqrt(T x) {
  return CtSearchSqrt<T>(x, 0, x / 2 + 1);
}

template <typename T>
T Sqrt(T x) {
  return std::sqrt(x);
}

// https://www.lomont.org/papers/2003/InvSqrt.pdf
constexpr f32 FastInvSqrt(f32 x) {
  union {
    f32 f;
    u32 i;
  } conv{x};

  conv.i = 0x5f375a86 - (conv.i >> 1);            // Initial guess y0.
  conv.f *= 1.5f - (x * 0.5f * conv.f * conv.f);  // Newton step.
  return conv.f;
}

template <typename T>
constexpr T Abs(T x) {
  if (x >= 0) {
    return x;
  }

  return -x;
}
}  // namespace math
}  // namespace comet

#endif  // COMET_COMET_MATH_MATH_COMMONS_H_
