// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "plane.h"

#include "math_commons.h"

namespace comet {
namespace math {
Plane::Plane(const Vec3& point, const Vec3& normal) {
  SetNormal(normal);
  SetDistance(point);
}

Plane::Plane(f32 distance, const Vec3& normal) : distance_{distance} {
  SetNormal(normal);
}

void Plane::SetNormal(const Vec3& normal) {
  normal_ = GetNormalizedCopy(normal);
}

void Plane::SetDistance(f32 distance) noexcept { distance_ = distance; }

void Plane::SetDistance(const Vec3& point) { distance_ = Dot(-normal_, point); }

const Vec3& Plane::GetNormal() const noexcept { return normal_; }

f32 Plane::GetDistance() const noexcept { return distance_; }

f32 GetSignedDistance(const Plane& plane, const Vec3& point) {
  return Dot(plane.GetNormal(), point) + plane.GetDistance();
}

bool Intersect(const Plane& p1, const Plane& p2, Vec3& point, Vec3& vector) {
  const auto& n1{p1.GetNormal()};
  const auto& n2{p2.GetNormal()};

  vector = Cross(n1, n2);
  const auto determinant{Dot(vector, vector)};

  if (Abs(determinant) <= kF32Min) {
    return false;
  }

  point = (Cross(vector, n2) * p1.GetDistance() +
           Cross(n1, vector) * p2.GetDistance()) /
          determinant;
  return true;
}

bool Intersect(const Plane& p1, const Plane& p2, const Plane& p3, Vec3& point) {
  const auto& n1{p1.GetNormal()};
  const auto& n2{p2.GetNormal()};
  const auto& n3{p3.GetNormal()};

  const auto cross_n1_n2{Cross(n1, n2)};
  const auto determinant{Dot(cross_n1_n2, n3)};

  if (Abs(determinant) <= kF32Min) {
    return false;
  }

  point = (Cross(n3, n2) * p1.GetDistance() + Cross(n1, n3) * p2.GetDistance() -
           cross_n1_n2 * p3.GetDistance()) /
          determinant;
  return true;
}
}  // namespace math
}  // namespace comet
