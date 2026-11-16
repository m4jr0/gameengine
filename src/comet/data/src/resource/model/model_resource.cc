// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
#include "comet/core/memory/memory_utils.h"
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
namespace packing_internal {
template <typename MeshResourceT>
void PackMeshResource(const MeshResourceT& mesh, u8* buffer, usize& cursor) {
  const auto vertex_count{mesh.vertices.GetSize()};
  const auto index_count{mesh.indices.GetSize()};

  memory::CopyMemory(&buffer[cursor], &mesh.resource_id, sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&buffer[cursor], &mesh.internal_id, sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&buffer[cursor], &mesh.type, sizeof(geometry::MeshType));
  cursor += sizeof(geometry::MeshType);

  memory::CopyMemory(&buffer[cursor], &mesh.material_resource_id,
                     sizeof(MaterialResourceId));
  cursor += sizeof(MaterialResourceId);

  memory::CopyMemory(&buffer[cursor], &mesh.transform, sizeof(math::Mat4));
  cursor += sizeof(math::Mat4);

  memory::CopyMemory(&buffer[cursor], &mesh.local_center, sizeof(math::Vec3));
  cursor += sizeof(math::Vec3);

  memory::CopyMemory(&buffer[cursor], &mesh.local_max_extents,
                     sizeof(math::Vec3));
  cursor += sizeof(math::Vec3);

  memory::CopyMemory(&buffer[cursor], &mesh.parent_id, sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&buffer[cursor], &vertex_count, sizeof(vertex_count));
  cursor += sizeof(vertex_count);

  if (vertex_count > 0) {
    const auto vertex_total_size{sizeof(geometry::SkinnedVertex) *
                                 vertex_count};
    memory::CopyMemory(&buffer[cursor], mesh.vertices.GetData(),
                       vertex_total_size);
    cursor += vertex_total_size;
  }

  memory::CopyMemory(&buffer[cursor], &index_count, sizeof(index_count));
  cursor += sizeof(index_count);

  if (index_count > 0) {
    const auto index_total_size{sizeof(geometry::Index) * index_count};
    memory::CopyMemory(&buffer[cursor], mesh.indices.GetData(),
                       index_total_size);
    cursor += index_total_size;
  }
}
}

ResourceFile PackStaticModelResource(
    const StaticModelResource& resource, CompressionMode compression_mode, memory::Allocator* allocator) {
  COMET_ASSERT(allocator != nullptr, "resource::PackStaticModelResource", "allocator is null");
  COMET_ASSERT(resource.type_id == StaticModelResource::kResourceTypeId,
               "StaticModelResourceHandler::Pack",
               "static model resource type id is invalid", "resource_type_id",
               resource.type_id, "expected_type_id",
               StaticModelResource::kResourceTypeId);

  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = StaticModelResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{allocator};
  file.data = Array<u8>{allocator};

  Array<u8> data{allocator};
  data.Resize(GetModelSize(resource));

  auto* buffer{data.GetData()};
  usize cursor{0};

  memory::CopyMemory(&buffer[cursor], &resource.id, sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&buffer[cursor], &resource.type_id,
                     sizeof(ResourceTypeId));
  cursor += sizeof(ResourceTypeId);

  const auto mesh_count{resource.meshes.GetSize()};
  memory::CopyMemory(&buffer[cursor], &mesh_count, sizeof(mesh_count));
  cursor += sizeof(mesh_count);

  for (const auto& mesh : resource.meshes) {
    packing_internal::PackMeshResource(mesh, buffer, cursor);
  }

  COMET_ASSERT(cursor == data.GetSize(), "StaticModelResourceHandler::Pack",
               "packed static model size mismatch", "cursor", cursor,
               "data_size", data.GetSize());

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

ResourceFile PackSkeletalModelResource(
    const SkeletalModelResource& resource, CompressionMode compression_mode, memory::Allocator* allocator) {
  COMET_ASSERT(allocator != nullptr, "resource::PackSkeletalModelResource", "allocator is null");
  COMET_ASSERT(resource.type_id == SkeletalModelResource::kResourceTypeId,
               "SkeletalModelResourceHandler::Pack",
               "skeletal model resource type id is invalid", "resource_type_id",
               resource.type_id, "expected_type_id",
               SkeletalModelResource::kResourceTypeId);

  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = SkeletalModelResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{allocator};
  file.data = Array<u8>{allocator};

  Array<u8> data{allocator};
  data.Resize(GetModelSize(resource));

  auto* buffer{data.GetData()};
  usize cursor{0};

  memory::CopyMemory(&buffer[cursor], &resource.id, sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&buffer[cursor], &resource.type_id,
                     sizeof(ResourceTypeId));
  cursor += sizeof(ResourceTypeId);

  memory::CopyMemory(&buffer[cursor], &resource.skeleton_id,
                     sizeof(geometry::SkeletonId));
  cursor += sizeof(geometry::SkeletonId);

  const auto mesh_count{resource.meshes.GetSize()};
  memory::CopyMemory(&buffer[cursor], &mesh_count, sizeof(mesh_count));
  cursor += sizeof(mesh_count);

  for (const auto& mesh : resource.meshes) {
    packing_internal::PackMeshResource(mesh, buffer, cursor);
  }

  COMET_ASSERT(cursor == data.GetSize(), "SkeletalModelResourceHandler::Pack",
               "packed skeletal model size mismatch", "cursor", cursor,
               "data_size", data.GetSize());

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

ResourceFile PackSkeletonResource(const SkeletonResource& resource,
                                           CompressionMode compression_mode, memory::Allocator* allocator) {
  COMET_ASSERT(allocator != nullptr, "resource::PackSkeletonResource", "allocator is null");
  COMET_ASSERT(resource.type_id == SkeletonResource::kResourceTypeId,
               "SkeletonResourceHandler::Pack",
               "skeleton resource type id is invalid", "resource_type_id",
               resource.type_id, "expected_type_id",
               SkeletonResource::kResourceTypeId);

  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = SkeletonResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{allocator};
  file.data = Array<u8>{allocator};

  Array<u8> data{allocator};
  data.Resize(GetSkeletonSize(resource));

  auto* buffer{data.GetData()};
  usize cursor{0};

  memory::CopyMemory(&buffer[cursor], &resource.id, sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&buffer[cursor], &resource.type_id,
                     sizeof(ResourceTypeId));
  cursor += sizeof(ResourceTypeId);

  memory::CopyMemory(&buffer[cursor], &resource.skeleton.id,
                     sizeof(geometry::SkeletonId));
  cursor += sizeof(geometry::SkeletonId);

  const auto joint_count{resource.skeleton.joints.GetSize()};
  memory::CopyMemory(&buffer[cursor], &joint_count, sizeof(joint_count));
  cursor += sizeof(joint_count);

  for (const auto& joint : resource.skeleton.joints) {
    memory::CopyMemory(&buffer[cursor], &joint.id,
                       sizeof(geometry::SkeletonJointId));
    cursor += sizeof(geometry::SkeletonJointId);

    memory::CopyMemory(&buffer[cursor], &joint.parent_index,
                       sizeof(geometry::SkeletonJointIndex));
    cursor += sizeof(geometry::SkeletonJointIndex);

    memory::CopyMemory(&buffer[cursor], &joint.bind_pose_inv,
                       sizeof(math::Mat4));
    cursor += sizeof(math::Mat4);
  }

  COMET_ASSERT(cursor == data.GetSize(), "SkeletonResourceHandler::Pack",
               "packed skeleton size mismatch", "cursor", cursor, "data_size",
               data.GetSize());

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

}  // namespace resource
}  // namespace comet