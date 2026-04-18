// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
#define COMET_COMET_RESOURCE_MODEL_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/geometry/geometry_type.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/resource.h"
#include "comet/resource/resource_id.h"
#include "comet/resource/runtime/loaded_resource_handle.h"

namespace comet {
namespace resource {
struct StaticModelResourceTag {};
struct SkeletalModelResourceTag {};
struct SkeletonResourceTag {};

using StaticModelResourceId = ResourceIdT<StaticModelResourceTag>;
using SkeletalModelResourceId = ResourceIdT<SkeletalModelResourceTag>;
using SkeletonResourceId = ResourceIdT<SkeletonResourceTag>;

using StaticModelResourceHandle = LoadedResourceHandle<StaticModelResourceTag>;
using SkeletalModelResourceHandle =
    LoadedResourceHandle<SkeletalModelResourceTag>;
using SkeletonResourceHandle = LoadedResourceHandle<SkeletonResourceTag>;

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
  using Handle = StaticModelResourceHandle;
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
  using Handle = SkeletalModelResourceHandle;
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
  using Handle = SkeletonResourceHandle;
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

#endif  // COMET_COMET_RESOURCE_MODEL_RESOURCE_H_