// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_RESOURCE_MODEL_MODEL_RESOURCE_H_
#define COMET_DATA_RESOURCE_MODEL_MODEL_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/container/array.h"
#include "comet/data/geometry/mesh.h"
#include "comet/data/geometry/skeleton.h"
#include "comet/core/math/matrix.h"
#include "comet/core/math/vector.h"
#include "comet/data/resource/resource_file.h"
#include "comet/data/resource/material/material_resource.h"
#include "comet/data/resource/resource.h"
#include "comet/data/resource/resource_id.h"

namespace comet {
namespace resource {
struct StaticModelResourceTag {};
struct SkeletalModelResourceTag {};
struct SkeletonResourceTag {};

using StaticModelResourceId = ResourceIdT<StaticModelResourceTag>;
using SkeletalModelResourceId = ResourceIdT<SkeletalModelResourceTag>;
using SkeletonResourceId = ResourceIdT<SkeletonResourceTag>;

struct MeshResource : InternalResource {
  geometry::MeshType type{geometry::MeshType::Unknown};
  MaterialResourceId material_resource_id{};
  math::Mat4 transform{1.0f};
  math::Vec3 local_center{.0f};
  math::Vec3 local_max_extents{.0f};
  RawResourceId parent_id{kInvalidRawResourceId};
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
  using Id = StaticModelResourceId;
  using TypeId = ResourceTypeId;
  using TypeName = ResourceTypeName;

  static constexpr TypeName kResourceTypeName{"static_model"};
  static const TypeId kResourceTypeId;

  StaticModelResourceDescr descr{};
  Array<StaticMeshResource> meshes{};

  Id GetId() const noexcept;
};

struct SkeletalModelResourceDescr {
  // TODO(m4jr0): Add description.
  u8 empty{0};
};

struct SkeletalModelResource : Resource {
  using Id = SkeletalModelResourceId;
  using TypeId = ResourceTypeId;
  using TypeName = ResourceTypeName;

  static constexpr TypeName kResourceTypeName{"skeletal_model"};
  static const TypeId kResourceTypeId;

  SkeletalModelResourceDescr descr{};
  geometry::SkeletonId skeleton_id{geometry::kInvalidSkeletonId};
  Array<SkinnedMeshResource> meshes{};

  Id GetId() const noexcept;
};

struct SkeletonResourceDescr {
  // TODO(m4jr0): Add description.
  u8 empty{0};
};

struct SkeletonResource : Resource {
  using Id = SkeletonResourceId;
  using TypeId = ResourceTypeId;
  using TypeName = ResourceTypeName;

  static constexpr TypeName kResourceTypeName{"skeleton"};
  static const TypeId kResourceTypeId;

  SkeletonResourceDescr descr{};
  geometry::Skeleton skeleton{};

  Id GetId() const noexcept;
};

usize GetMeshSize(const StaticMeshResource& resource);
usize GetModelSize(const StaticModelResource& resource);

usize GetMeshSize(const SkinnedMeshResource& resource);
usize GetModelSize(const SkeletalModelResource& resource);

usize GetSkeletonJointSize();
usize GetSkeletonSize(const SkeletonResource& resource);
}  // namespace resource
}  // namespace comet

#endif  // COMET_DATA_RESOURCE_MODEL_MODEL_RESOURCE_H_