// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "quaternion.h"

#include "comet/math/geometry.h"
#include "comet/math/vector.h"

namespace comet {
namespace math {
Quat& Normalize(Quat& quaternion) {
  auto* vector{reinterpret_cast<Vec4*>(&quaternion)};
  Normalize(*vector);
  return quaternion;
}

Quat GetNormalizedCopy(const Quat& quaternion) {
  auto copy{quaternion};
  Normalize(copy);
  return copy;
}

Quat GetQuaternionRotation(f32 angle, const Vec3& axis) {
  auto theta{angle / 2};
  auto sin_theta{Sin(theta)};

  Quat quaternion{Cos(theta), axis.x * sin_theta, axis.y * sin_theta,
                  axis.z * sin_theta};
  return quaternion;
}
}  // namespace math
}  // namespace comet
