// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_MATH_INTERPOLATION_H_
#define COMET_CORE_MATH_INTERPOLATION_H_

#include "comet/core/essentials.h"
#include "comet/math/geometry.h"
#include "comet/math/math_scalar.h"
#include "comet/math/quaternion.h"

namespace comet {
namespace math {
template <typename T1, typename T2>
inline T1 Lerp(T1 a, T1 b, T2 t) {
  return a + (b - a) * t;
}

// https://gitlab.com/bztsrc/slerp-opt
template <typename T>
Quat Slerp(const Quat& quat_a, const Quat& quat_b, T t) {
#ifdef COMET_USE_LERP_FOR_SLERP
  const auto result{Lerp(quat_a, quat_b, t)};
  return Normalize(result);
#else
  auto a{1.0 - t};
  auto b{t};
  const auto d{quat_a.x * quat_b.x + quat_a.y * quat_b.y + quat_a.z * quat_b.z +
               quat_a.w * quat_b.w};
  auto c{Abs(d)};

  if (c < .999) {
    c = Acos(c);
    b = 1 / Sin(c);
    a = Sin(a * c) * b;
    b *= Sin(t * c);

    if (d < 0) {
      b = -b;
    }
  }

  return Quat{static_cast<f32>(quat_a.w * a + quat_b.w * b),
              static_cast<f32>(quat_a.x * a + quat_b.x * b),
              static_cast<f32>(quat_a.y * a + quat_b.y * b),
              static_cast<f32>(quat_a.z * a + quat_b.z * b)};
#endif  // COMET_USE_LERP_FOR_SLERP
}
}  // namespace math
}  // namespace comet

#endif  // COMET_CORE_MATH_INTERPOLATION_H_
