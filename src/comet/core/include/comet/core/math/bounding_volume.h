// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_MATH_BOUNDING_VOLUME_H_
#define COMET_CORE_MATH_BOUNDING_VOLUME_H_

#include "comet/core/essentials.h"
#include "comet/core/math/matrix.h"
#include "comet/core/math/plane.h"
#include "comet/core/math/vector.h"

namespace comet {
namespace math {
struct Aabb {
  Vec3 center{.0f};
  Vec3 extents{.0f};
};

struct Sphere {
  Vec3 center{.0f};
  f32 radius{0};
};

bool IsAabbInOrOnPlane(const Plane& plane, const Aabb& aabb);
bool IsSphereInOrOnPlane(const Plane& plane, const Sphere& sphere);

Aabb GenerateGlobalAabb(const Aabb& local_aabb, const Mat4& global);
Aabb GenerateGlobalAabb(const Vec3& local_center, const Vec3& local_extents,
                        const Mat4& global);
}  // namespace math
}  // namespace comet

#endif  // COMET_CORE_MATH_BOUNDING_VOLUME_H_
