// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "bounding_volume.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/math/math_scalar.h"

namespace comet {
namespace math {
bool IsAabbInOrOnPlane(const Plane& plane, const Aabb& aabb) {
  const auto& normal{plane.GetNormal()};
  const auto radius{aabb.extents.x * Abs(normal.x) +
                    aabb.extents.y * Abs(normal.y) +
                    aabb.extents.z * Abs(normal.z)};
  return -radius <= GetSignedDistance(plane, aabb.center);
}

bool IsSphereInOrOnPlane(const Plane& plane, const Sphere& sphere) {
  return GetSignedDistance(plane, sphere.center) > -sphere.radius;
}

Aabb GenerateGlobalAabb(const Aabb& local_aabb, const Mat4& global) {
  return GenerateGlobalAabb(local_aabb.center, local_aabb.extents, global);
}

Aabb GenerateGlobalAabb(const Vec3& local_center, const Vec3& local_extents,
                        const Mat4& global) {
  const auto local_max_extents{local_center + local_extents};
  const auto local_min_extents{local_center - local_extents};
  Vec3 global_min_extents{.0f};
  Vec3 global_max_extents{.0f};

  for (u8 i{0}; i < 3; ++i) {
    for (u8 j{0}; j < 3; ++j) {
      const auto from_min{global[j][i] * local_min_extents[j]};
      const auto from_max{global[j][i] * local_max_extents[j]};

      if (from_min < from_max) {
        global_min_extents[i] += from_min;
        global_max_extents[i] += from_max;
      } else {
        global_min_extents[i] += from_max;
        global_max_extents[i] += from_min;
      }
    }
  }

  Aabb aabb{};
  aabb.center = (global_min_extents + global_max_extents) * .5f;
  aabb.extents = global_max_extents - aabb.center;
  return aabb;
}
}  // namespace math
}  // namespace comet
