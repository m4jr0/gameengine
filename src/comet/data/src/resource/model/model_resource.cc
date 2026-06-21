// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/resource/model/model_resource.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/id/string_id.h"
#include "comet/core/id/string_id_allocator.h"

namespace comet {
namespace resource {
const StaticModelResource::TypeId StaticModelResource::kResourceTypeId{
    COMET_STRING_ID(StaticModelResource::kResourceTypeName.data())};

const SkeletalModelResource::TypeId SkeletalModelResource::kResourceTypeId{
    COMET_STRING_ID(SkeletalModelResource::kResourceTypeName.data())};

const SkeletonResource::TypeId SkeletonResource::kResourceTypeId{
    COMET_STRING_ID(SkeletonResource::kResourceTypeName.data())};

StaticModelResource::Id StaticModelResource::GetId() const noexcept {
  return Id{id};
}

SkeletalModelResource::Id SkeletalModelResource::GetId() const noexcept {
  return Id{id};
}

SkeletonResource::Id SkeletonResource::GetId() const noexcept { return Id{id}; }

usize GetMeshSize(const StaticMeshResource& resource) {
  const auto vertex_count{resource.vertices.GetSize()};
  const auto index_count{resource.indices.GetSize()};

  return sizeof(RawResourceId) +       // resource_id
         sizeof(RawResourceId) +       // internal_id
         sizeof(geometry::MeshType) +  // type
         sizeof(MaterialResourceId) +  // material_resource_id
         sizeof(math::Mat4) +          // transform
         sizeof(math::Vec3) +          // local_center
         sizeof(math::Vec3) +          // local_max_extents
         sizeof(RawResourceId) +       // parent_id
         sizeof(vertex_count) +        // vertex_count
         vertex_count * sizeof(geometry::SkinnedVertex) +
         sizeof(index_count) +  // index_count
         index_count * sizeof(geometry::Index);
}

usize GetModelSize(const StaticModelResource& resource) {
  auto size{sizeof(RawResourceId) + sizeof(ResourceTypeId) + sizeof(usize)};

  for (const auto& mesh : resource.meshes) {
    size += GetMeshSize(mesh);
  }

  return size;
}

usize GetMeshSize(const SkinnedMeshResource& resource) {
  const auto vertex_count{resource.vertices.GetSize()};
  const auto index_count{resource.indices.GetSize()};

  return sizeof(RawResourceId) +       // resource_id
         sizeof(RawResourceId) +       // internal_id
         sizeof(geometry::MeshType) +  // type
         sizeof(MaterialResourceId) +  // material_resource_id
         sizeof(math::Mat4) +          // transform
         sizeof(math::Vec3) +          // local_center
         sizeof(math::Vec3) +          // local_max_extents
         sizeof(RawResourceId) +       // parent_id
         sizeof(vertex_count) +        // vertex_count
         vertex_count * sizeof(geometry::SkinnedVertex) +
         sizeof(index_count) +  // index_count
         index_count * sizeof(geometry::Index);
}

usize GetModelSize(const SkeletalModelResource& resource) {
  auto size{sizeof(RawResourceId) + sizeof(ResourceTypeId) +
            sizeof(geometry::SkeletonId) + sizeof(usize)};

  for (const auto& mesh : resource.meshes) {
    size += GetMeshSize(mesh);
  }

  return size;
}

usize GetSkeletonJointSize() {
  return sizeof(geometry::SkeletonJointId) +
         sizeof(geometry::SkeletonJointIndex) + sizeof(math::Mat4);
}

usize GetSkeletonSize(const SkeletonResource& resource) {
  return sizeof(RawResourceId) + sizeof(ResourceTypeId) +
         sizeof(geometry::SkeletonId) + sizeof(usize) +
         resource.skeleton.joints.GetSize() * GetSkeletonJointSize();
}
}  // namespace resource
}  // namespace comet