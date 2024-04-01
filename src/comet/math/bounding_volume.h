// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_UTILS_MATH_BOUNDING_VOLUME_H_
#define COMET_COMET_UTILS_MATH_BOUNDING_VOLUME_H_

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/math/plane.h"
#include "comet/math/vector.h"

namespace comet {
namespace math {
struct Aabb {
  Vec3 center{0.0f};
  Vec3 extents{0.0f};
};

struct Sphere {
  Vec3 center{0.0f};
  f32 radius{0};
};

bool IsAabbInOrOnPlane(const Plane& plane, const Aabb& aabb);
bool IsSphereInOrOnPlane(const Plane& plane, const Sphere& sphere);
math::Aabb GenerateGlobalAabb(const Aabb& local_aabb, const Mat4& global);

math::Aabb GenerateGlobalAabb(const Vec3& local_center,
                              const Vec3& local_extents, const Mat4& global);
}  // namespace math
}  // namespace comet

#endif  // COMET_COMET_UTILS_MATH_BOUNDING_VOLUME_H_
