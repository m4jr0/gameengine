// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_MATH_MATH_COMMON_H_
#define COMET_COMET_MATH_MATH_COMMON_H_

// External. ///////////////////////////////////////////////////////////////////
#include <cmath>
////////////////////////////////////////////////////////////////////////////////

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
T Abs(T x) {
  if (x >= 0) {
    return x;
  }

  return -x;
}

template <typename T>
s8 Sign(T x) {
  if (x >= 0) {
    return 1;
  }

  return -1;
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

template <typename T>
constexpr T Trunc(T x) {
  static_assert(std::is_floating_point_v<T>,
                "Trunc requires a floating-point type");

  return (x < T{0}) ? -static_cast<T>(static_cast<u64>(-x))
                    : static_cast<T>(static_cast<u64>(x));
}

template <typename T>
constexpr T Fmod(T x, T y) {
  static_assert(std::is_floating_point_v<T>,
                "Fmod requires a floating-point type");

  if (y == T{0}) {
    return T{0};
  }

  return x - Trunc(x / y) * y;
}

template <typename T>
constexpr T Wrap(T value, T period) {
  static_assert(std::is_floating_point_v<T>,
                "Wrap requires a floating-point type");

  if (period <= T{0}) {
    return T{0};
  }

  value = Fmod(value, period);

  if (value < T{0}) {
    value += period;
  }

  return value;
}

template <typename T, typename Exp,
          typename = std::enable_if_t<std::is_integral_v<Exp>>>
constexpr T Pow(T base, Exp exp) {
  if constexpr (std::is_signed_v<Exp>) {
    if (exp < 0) {
      static_assert(std::is_floating_point_v<T>,
                    "Negative exponents require a floating-point base!");

      if (base == T{0}) {
        return T{0};
      }

      return T{1} / Pow(base, static_cast<std::make_unsigned_t<Exp>>(-exp));
    }
  }

  using UExp =
      std::conditional_t<std::is_signed_v<Exp>, std::make_unsigned_t<Exp>, Exp>;

  UExp e{static_cast<UExp>(exp)};
  T result{1};

  while (e > 0) {
    if ((e & 1) != 0) {
      result *= base;
    }

    e >>= 1;
    if (e != 0) {
      base *= base;
    }
  }

  return result;
}

template <typename Base, typename Exp,
          typename = std::enable_if_t<std::is_floating_point_v<Base> ||
                                      std::is_floating_point_v<Exp>>>
auto Pow(Base base, Exp exp) {
  return std::pow(base, exp);
}
}  // namespace math
}  // namespace comet

#endif  // COMET_COMET_MATH_MATH_COMMON_H_
