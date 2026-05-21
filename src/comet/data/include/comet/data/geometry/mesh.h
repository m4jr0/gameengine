// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_GEOMETRY_MESH_H_
#define COMET_DATA_GEOMETRY_MESH_H_

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/handle.h"
#include "comet/geometry/type/skeleton.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"

namespace comet {
namespace geometry {
struct Vertex {
  math::Vec3 position{};
  math::Vec3 normal{};
  math::Vec4 tangent{};
  math::Vec2 uv{};
  math::Vec4 color{};
};

struct SkinnedVertex {
  math::Vec3 position{};
  math::Vec3 normal{};
  math::Vec4 tangent{};
  math::Vec2 uv{};
  math::Vec4 color{};
  SkeletonJointIndex joint_indices[kMaxSkeletonJointCount]{
      kInvalidSkeletonJointIndex, kInvalidSkeletonJointIndex,
      kInvalidSkeletonJointIndex, kInvalidSkeletonJointIndex};
  JointWeight joint_weights[kMaxSkeletonJointCount]{};
};

using Index = u32;

using MeshId = u64;
constexpr auto kInvalidMeshId{static_cast<MeshId>(-1)};

enum class MeshType : u8 { Unknown = 0, Static, Skinned };
}  // namespace geometry
}  // namespace comet

#endif  // COMET_DATA_GEOMETRY_MESH_H_