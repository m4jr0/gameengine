// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_UTILS_MATH_MATH_H_
#define COMET_COMET_UTILS_MATH_MATH_H_

namespace comet {
namespace utils {
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
}  // namespace math
}  // namespace utils
}  // namespace comet

#endif  // COMET_COMET_UTILS_MATH_MATH_H_
