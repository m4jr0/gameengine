// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GEOMETRY_GEOMETRY_COMMON_H_
#define COMET_COMET_GEOMETRY_GEOMETRY_COMMON_H_

#include <atomic>

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/string_id.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"

namespace comet {
namespace geometry {
struct Vertex {
  math::Vec3 position{};
  math::Vec3 normal{};
  math::Vec3 tangent{};
  math::Vec3 bitangent{};
  math::Vec2 uv{};
  math::Vec4 color{};
};

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

struct SkinnedVertex : Vertex {
  SkeletonJointIndex joint_indices[kMaxSkeletonJointCount]{
      kInvalidSkeletonJointIndex, kInvalidSkeletonJointIndex,
      kInvalidSkeletonJointIndex, kInvalidSkeletonJointIndex};
  JointWeight joint_weights[kMaxSkeletonJointCount]{};
};

using Index = u32;

using MeshId = u64;
constexpr auto kInvalidMeshId{static_cast<MeshId>(-1)};

enum class MeshType : u8 { Unknown = 0, Static, Skinned };

const schar* GetMeshTypeLabel(MeshType mesh_type);

struct Mesh {
  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> needs to be always lock-free. Unsupported "
                "architecture");

  bool is_dirty{false};
  std::atomic<usize> ref_count{0};
  MeshId id{kInvalidMeshId};
  MeshType type{geometry::MeshType::Unknown};
  math::Mat4 transform{1.0f};
  math::Vec3 local_center{0.0f};
  math::Vec3 local_max_extents{0.0f};
  Array<geometry::Index> indices{};
  Array<geometry::SkinnedVertex> vertices{};
};
}  // namespace geometry
}  // namespace comet

#endif  // COMET_COMET_GEOMETRY_GEOMETRY_COMMON_H_
