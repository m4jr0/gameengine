// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_MATH_PLANE_H_
#define COMET_COMET_MATH_PLANE_H_

#include "comet/core/essentials.h"
#include "comet/math/vector.h"

namespace comet {
namespace math {
class Plane {
 public:
  Plane() = default;
  Plane(const Vec3& point, const Vec3& normal);
  Plane(f32 distance, const Vec3& normal);

  void SetNormal(const Vec3& normal);
  void SetDistance(f32 distance) noexcept;
  void SetDistance(const Vec3& point);
  const Vec3& GetNormal() const noexcept;
  f32 GetDistance() const noexcept;

 private:
  Vec3 normal_{0.0f};
  f32 distance_{0.0f};
};

f32 GetSignedDistance(const Plane& plane, const Vec3& point);
bool Intersect(const Plane& p1, const Plane& p2, Vec3& point, Vec3& vector);
bool Intersect(const Plane& p1, const Plane& p2, const Plane& p3, Vec3& point);
}  // namespace math
}  // namespace comet

#endif  // COMET_COMET_MATH_PLANE_H_
