// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_MATH_MATH_COMMONS_H_
#define COMET_COMET_MATH_MATH_COMMONS_H_

#include <cmath>

#include "comet/core/essentials.h"

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

template <class T>
const T& Min(const T& a, const T& b) {
  return (a < b) ? a : b;
}

template <class T>
const T& Max(const T& a, const T& b) {
  return (a > b) ? a : b;
}

template <class T>
const T& Clamp(const T& x, const T& min, const T& max) {
  return Min(Max(x, min), max);
}

template <class T, typename = std::enable_if_t<std::is_integral_v<T>>>
const T& Floor(const T& x) {
  return x;
}

constexpr s32 Floor(f32 x) { return static_cast<s32>(x); }

constexpr s64 Floor(f64 x) { return static_cast<s64>(x); }

template <class T, typename = std::enable_if_t<std::is_integral_v<T>>>
const T& Ceil(const T& x) {
  return x;
}

constexpr s32 Ceil(f32 x) {
  const auto i{static_cast<s32>(x)};
  return x > i ? i + 1 : i;
}

constexpr s64 Ceil(f64 x) {
  const auto i{static_cast<s64>(x)};
  return x > i ? i + 1 : i;
}

template <class T>
const T Log2(const T& x, const T& i = 0) {
  return (x < 2) ? i : Log2(x / 2, i + 1);
}
}  // namespace math
}  // namespace comet

#endif  // COMET_COMET_MATH_MATH_COMMONS_H_
