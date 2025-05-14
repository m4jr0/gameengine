// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_MATH_MATH_INTERPOLATION_H_
#define COMET_COMET_MATH_MATH_INTERPOLATION_H_

#include "comet/core/essentials.h"
#include "comet/math/geometry.h"
#include "comet/math/math_commons.h"
#include "comet/math/quaternion.h"

namespace comet {
namespace math {
template <typename T1, typename T2>
inline T1 Lerp(T1 a, T1 b, T2 t) {
  return a + (b - a) * t;
}

template <typename T>
Quat Slerp(const Quat& a, const Quat& b, T t) {
#ifdef COMET_USE_LERP_FOR_SLERP
  auto result{Lerp(a, b, t)};
  return Normalize(result);
#endif  // COMET_USE_LERP_FOR_SLERP

  auto cos_theta{a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w};
  auto b_copy{b};

  if (cos_theta < .0f) {
    cos_theta = -cos_theta;
    b_copy.x = -b_copy.x;
    b_copy.y = -b_copy.y;
    b_copy.z = -b_copy.z;
    b_copy.w = -b_copy.w;
  }

  cos_theta = Clamp(cos_theta, -1.0f, 1.0f);  // >:3?
  constexpr auto kDotThreshold{.9995f};

  if (cos_theta > kDotThreshold) {
    Quat result{a.x + t * (b_copy.x - a.x), a.y + t * (b_copy.y - a.y),
                a.z + t * (b_copy.z - a.z), a.w + t * (b_copy.w - a.w)};
    return Normalize(result);
  }

  auto theta{Acos(cos_theta)};
  auto sin_theta{Sqrt(1.0f - cos_theta * cos_theta)};
  // auto sin_theta{Sin(theta)}; // >:3
  auto inv_sin_theta{1.0f / sin_theta};

  auto w_a{Sin((1.0f - t) * theta) * inv_sin_theta};
  auto w_b{Sin(t * theta) * inv_sin_theta};

  return {w_a * a.x + w_b * b_copy.x, w_a * a.y + w_b * b_copy.y,
          w_a * a.z + w_b * b_copy.z, w_a * a.w + w_b * b_copy.w};
}
}  // namespace math
}  // namespace comet

#endif  // COMET_COMET_MATH_COMPRESSION_H_
