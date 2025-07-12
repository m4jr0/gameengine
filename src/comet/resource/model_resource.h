// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
#define COMET_COMET_RESOURCE_MODEL_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/geometry/geometry_common.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
struct MeshResource : InternalResource {
  geometry::MeshType type{geometry::MeshType::Unknown};
  ResourceId material_id{kInvalidResourceId};
  math::Mat4 transform{1.0f};
  math::Vec3 local_center{0.0f};
  math::Vec3 local_max_extents{0.0f};
  ResourceId parent_id{kInvalidResourceId};
  Array<geometry::Index> indices{};
};

struct StaticMeshResource : MeshResource {
  Array<geometry::SkinnedVertex> vertices{};
};

struct SkinnedMeshResource : MeshResource {
  Array<geometry::SkinnedVertex> vertices{};
};

struct StaticModelResourceDescr {
  // TODO(m4jr0): Add description.
  u8 empty{0};
};

struct StaticModelResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  StaticModelResourceDescr descr{};
  Array<StaticMeshResource> meshes{};
};

struct SkeletalModelResourceDescr {
  // TODO(m4jr0): Add description.
  u8 empty{0};
};

struct SkeletalModelResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  SkeletalModelResourceDescr descr{};
  geometry::SkeletonId skeleton_id{geometry::kInvalidSkeletonId};
  Array<SkinnedMeshResource> meshes{};
};

struct SkeletonResourceDescr {
  // TODO(m4jr0): Add description.
  u8 empty{0};
};

struct SkeletonResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  SkeletonResourceDescr descr{};
  geometry::Skeleton skeleton{};
};

usize GetMeshSize(const StaticMeshResource& resource);
usize GetModelSize(const StaticModelResource& resource);
usize GetMeshSize(const SkinnedMeshResource& resource);
usize GetModelSize(const SkeletalModelResource& resource);
usize GetSkeletonJointSize();
usize GetSkeletonSize(const SkeletonResource& resource);
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
