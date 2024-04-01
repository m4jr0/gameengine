// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GEOMETRY_GEOMETRY_COMMON_H_
#define COMET_COMET_GEOMETRY_GEOMETRY_COMMON_H_

#include <vector>

#include "comet/core/essentials.h"
#include "comet/core/type/string_id.h"
#include "comet/math/matrix.h"
#include "comet/math/quaternion.h"
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

using JointIndex = u8;
constexpr auto kInvalidJointIndex{static_cast<JointIndex>(-1)};

struct Joint {
  math::Mat4x3 bind_pose_inv{};
  stringid::StringId id{stringid::kInvalidStringId};
  JointIndex parent{kInvalidJointIndex};
};

struct Skeleton {
  usize joint_count{0};
  std::vector<Joint> joints{};
};

struct JointPose {
  math::Quat rotation{};
  math::Vec3 translation{};
  f32 scale{1.0f};
};

struct SkeletonPose {
  Skeleton* skeleton{nullptr};
  JointPose* local_pose{nullptr};
  math::Mat4* global_pose{nullptr};
};

constexpr auto kJointIndexCount{4};
constexpr auto kJointWeightCount{4};

using JointWeight = f32;

struct SkinnedVertex : Vertex {
  JointIndex joint_indices[kJointIndexCount]{
      kInvalidJointIndex, kInvalidJointIndex, kInvalidJointIndex,
      kInvalidJointIndex};
  JointWeight joint_weights[kJointWeightCount]{};
};

using Index = u32;

using MeshId = u64;
constexpr auto kInvalidMeshId{static_cast<MeshId>(-1)};

enum class MeshType : u8 { Unknown = 0, Static, Skinned };

const schar* GetMeshTypeLabel(MeshType mesh_type);

struct Mesh {
  bool is_dirty{false};
  MeshId id{kInvalidMeshId};
  MeshType type{geometry::MeshType::Unknown};
  math::Mat4 transform{1.0f};
  math::Vec3 local_center{0.0f};
  math::Vec3 local_max_extents{0.0f};
  std::vector<geometry::Index> indices{};
  std::vector<geometry::Vertex> vertices{};
};
}  // namespace geometry
}  // namespace comet

#endif  // COMET_COMET_GEOMETRY_GEOMETRY_COMMON_H_
