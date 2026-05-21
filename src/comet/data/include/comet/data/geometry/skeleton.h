// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_GEOMETRY_SKELETON_H_
#define COMET_DATA_GEOMETRY_SKELETON_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/string_id.h"
#include "comet/math/matrix.h"

namespace comet {
namespace geometry {
using SkeletonJointId = stringid::StringId;
constexpr auto kInvalidSkeletonJointId{stringid::kInvalidStringId};

using SkeletonJointIndex = u16;
constexpr auto kInvalidSkeletonJointIndex{static_cast<SkeletonJointIndex>(-1)};

struct SkeletonJoint {
  SkeletonJointId id{kInvalidSkeletonJointId};
  SkeletonJointIndex parent_index{kInvalidSkeletonJointIndex};
  math::Mat4 bind_pose_inv{};
};

using SkeletonId = u32;
constexpr auto kInvalidSkeletonId{static_cast<SkeletonId>(-1)};

struct Skeleton {
  SkeletonId id{kInvalidSkeletonId};
  Array<SkeletonJoint> joints{};
};

constexpr SkeletonJointIndex kMaxSkeletonJointCount{4};

using JointWeight = f32;
}  // namespace geometry
}  // namespace comet

#endif  // COMET_DATA_GEOMETRY_SKELETON_H_