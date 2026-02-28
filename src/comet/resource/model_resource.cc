// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "model_resource.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type/array.h"

namespace comet {
namespace resource {
const ResourceTypeId StaticModelResource::kResourceTypeId{
    COMET_STRING_ID("static_model")};

const ResourceTypeId SkeletalModelResource::kResourceTypeId{
    COMET_STRING_ID("skeletal_model")};

const ResourceTypeId SkeletonResource::kResourceTypeId{
    COMET_STRING_ID("skeleton")};

usize GetMeshSize(const StaticMeshResource& resource) {
  constexpr auto kModelIdSize{sizeof(ResourceId)};
  constexpr auto kMeshIdSize{sizeof(ResourceId)};
  constexpr auto kMeshType{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  const auto kVertexCount{resource.vertices.GetSize()};
  const auto kIndexCount{resource.indices.GetSize()};

  const auto kVertexCountSize{sizeof(kVertexCount)};
  const auto kIndexCountSize{sizeof(kIndexCount)};

  constexpr auto kVertexSize{sizeof(geometry::SkinnedVertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  return kModelIdSize + kMeshIdSize + kMeshType + kMaterialIdSize +
         kTransformSize + kLocalCenterSize + kLocalMaxExtentsSize +
         kParentMeshIdSize + kVertexCountSize + kVertexCount * kVertexSize +
         kIndexCountSize + kIndexCount * kIndexSize;
}

usize GetModelSize(const StaticModelResource& resource) {
  usize size{sizeof(ResourceId) + sizeof(ResourceTypeId) + sizeof(usize)};

  for (const auto& mesh : resource.meshes) {
    size += GetMeshSize(mesh);
  }

  return size;
}

usize GetMeshSize(const SkinnedMeshResource& resource) {
  constexpr auto kModelIdSize{sizeof(ResourceId)};
  constexpr auto kMeshIdSize{sizeof(ResourceId)};
  constexpr auto kMeshType{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  const auto kVertexCount{resource.vertices.GetSize()};
  const auto kIndexCount{resource.indices.GetSize()};

  const auto kVertexCountSize{sizeof(kVertexCount)};
  const auto kIndexCountSize{sizeof(kIndexCount)};

  constexpr auto kVertexSize{sizeof(geometry::SkinnedVertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  return kModelIdSize + kMeshIdSize + kMeshType + kMaterialIdSize +
         kTransformSize + kLocalCenterSize + kLocalMaxExtentsSize +
         kParentMeshIdSize + kVertexCountSize + kVertexCount * kVertexSize +
         kIndexCountSize + kIndexCount * kIndexSize;
}

usize GetModelSize(const SkeletalModelResource& resource) {
  usize size{sizeof(ResourceId) + sizeof(ResourceTypeId) +
             sizeof(geometry::SkeletonId) + sizeof(usize)};

  for (const auto& mesh : resource.meshes) {
    size += GetMeshSize(mesh);
  }

  return size;
}

usize GetSkeletonJointSize() {
  return sizeof(geometry::SkeletonJointId) + sizeof(math::Mat4) +
         sizeof(geometry::SkeletonJointIndex);
}

usize GetSkeletonSize(const SkeletonResource& resource) {
  usize size{sizeof(ResourceId) + sizeof(ResourceTypeId) +
             sizeof(geometry::SkeletonId) + sizeof(usize)};

  auto joint_count{resource.skeleton.joints.GetSize()};

  for (usize i{0}; i < joint_count; ++i) {
    size += GetSkeletonJointSize();
  }

  return size;
}
}  // namespace resource
}  // namespace comet
