// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "model_resource_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/geometry/geometry_type.h"

namespace comet {
namespace resource {
namespace {
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

template <typename MeshResourceT>
void UnpackMeshResource(const u8* buffer, usize& cursor,
                        ResourceLifeSpan life_span,
                        memory::Allocator* allocator, MeshResourceT& mesh) {
  memory::CopyMemory(&mesh.resource_id, &buffer[cursor], sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&mesh.internal_id, &buffer[cursor], sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&mesh.type, &buffer[cursor], sizeof(geometry::MeshType));
  cursor += sizeof(geometry::MeshType);

  memory::CopyMemory(&mesh.material_resource_id, &buffer[cursor],
                     sizeof(MaterialResourceId));
  cursor += sizeof(MaterialResourceId);

  memory::CopyMemory(&mesh.transform, &buffer[cursor], sizeof(math::Mat4));
  cursor += sizeof(math::Mat4);

  memory::CopyMemory(&mesh.local_center, &buffer[cursor], sizeof(math::Vec3));
  cursor += sizeof(math::Vec3);

  memory::CopyMemory(&mesh.local_max_extents, &buffer[cursor],
                     sizeof(math::Vec3));
  cursor += sizeof(math::Vec3);

  memory::CopyMemory(&mesh.parent_id, &buffer[cursor], sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  usize vertex_count{0};
  memory::CopyMemory(&vertex_count, &buffer[cursor], sizeof(vertex_count));
  cursor += sizeof(vertex_count);

  mesh.vertices = Array<geometry::SkinnedVertex>{allocator};
  mesh.vertices.Resize(vertex_count);

  if (vertex_count > 0) {
    const auto vertex_total_size{sizeof(geometry::SkinnedVertex) *
                                 vertex_count};
    memory::CopyMemory(mesh.vertices.GetData(), &buffer[cursor],
                       vertex_total_size);
    cursor += vertex_total_size;
  }

  usize index_count{0};
  memory::CopyMemory(&index_count, &buffer[cursor], sizeof(index_count));
  cursor += sizeof(index_count);

  mesh.indices = Array<geometry::Index>{allocator};
  mesh.indices.Resize(index_count);

  if (index_count > 0) {
    const auto index_total_size{sizeof(geometry::Index) * index_count};
    memory::CopyMemory(mesh.indices.GetData(), &buffer[cursor],
                       index_total_size);
    cursor += index_total_size;
  }

  (void)life_span;
}
}  // namespace

StaticModelResourceHandler::StaticModelResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

ResourceFile StaticModelResourceHandler::Pack(
    const StaticModelResource& resource, CompressionMode compression_mode) {
  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = StaticModelResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{byte_allocator_};
  file.data = Array<u8>{byte_allocator_};

  Array<u8> data{byte_allocator_};
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
    PackMeshResource(mesh, buffer, cursor);
  }

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

void StaticModelResourceHandler::Unpack(const ResourceFile& file,
                                        ResourceLifeSpan life_span,
                                        StaticModelResource* resource) {
  UnpackPodResourceDescr<StaticModelResourceDescr>(file, resource->descr);

  Array<u8> data{byte_allocator_};
  UnpackResourceData(file, data);

  const auto* buffer{data.GetData()};
  usize cursor{0};

  memory::CopyMemory(&resource->id, &buffer[cursor], sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&resource->type_id, &buffer[cursor],
                     sizeof(ResourceTypeId));
  cursor += sizeof(ResourceTypeId);

  COMET_ASSERT(resource->type_id == StaticModelResource::kResourceTypeId,
               "Unpacked resource is not a static model!");

  usize mesh_count{0};
  memory::CopyMemory(&mesh_count, &buffer[cursor], sizeof(mesh_count));
  cursor += sizeof(mesh_count);

  auto* allocator{ResolveAllocator(byte_allocator_, life_span)};
  resource->meshes = Array<StaticMeshResource>{allocator, mesh_count};

  for (usize i{0}; i < mesh_count; ++i) {
    auto& mesh{resource->meshes.EmplaceBack()};
    UnpackMeshResource(buffer, cursor, life_span, allocator, mesh);
  }

  resource->life_span = life_span;
}

SkeletalModelResourceHandler::SkeletalModelResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

ResourceFile SkeletalModelResourceHandler::Pack(
    const SkeletalModelResource& resource, CompressionMode compression_mode) {
  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = SkeletalModelResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{byte_allocator_};
  file.data = Array<u8>{byte_allocator_};

  Array<u8> data{byte_allocator_};
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
    PackMeshResource(mesh, buffer, cursor);
  }

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

void SkeletalModelResourceHandler::Unpack(const ResourceFile& file,
                                          ResourceLifeSpan life_span,
                                          SkeletalModelResource* resource) {
  UnpackPodResourceDescr<SkeletalModelResourceDescr>(file, resource->descr);

  Array<u8> data{byte_allocator_};
  UnpackResourceData(file, data);

  const auto* buffer{data.GetData()};
  usize cursor{0};

  memory::CopyMemory(&resource->id, &buffer[cursor], sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&resource->type_id, &buffer[cursor],
                     sizeof(ResourceTypeId));
  cursor += sizeof(ResourceTypeId);

  memory::CopyMemory(&resource->skeleton_id, &buffer[cursor],
                     sizeof(geometry::SkeletonId));
  cursor += sizeof(geometry::SkeletonId);

  COMET_ASSERT(resource->type_id == SkeletalModelResource::kResourceTypeId,
               "Unpacked resource is not a skeletal model!");

  usize mesh_count{0};
  memory::CopyMemory(&mesh_count, &buffer[cursor], sizeof(mesh_count));
  cursor += sizeof(mesh_count);

  auto* allocator{ResolveAllocator(byte_allocator_, life_span)};
  resource->meshes = Array<SkinnedMeshResource>{allocator, mesh_count};

  for (usize i{0}; i < mesh_count; ++i) {
    auto& mesh{resource->meshes.EmplaceBack()};
    UnpackMeshResource(buffer, cursor, life_span, allocator, mesh);
  }

  resource->life_span = life_span;
}

SkeletonResourceHandler::SkeletonResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

ResourceFile SkeletonResourceHandler::Pack(const SkeletonResource& resource,
                                           CompressionMode compression_mode) {
  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = SkeletonResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{byte_allocator_};
  file.data = Array<u8>{byte_allocator_};

  Array<u8> data{byte_allocator_};
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

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

void SkeletonResourceHandler::Unpack(const ResourceFile& file,
                                     ResourceLifeSpan life_span,
                                     SkeletonResource* resource) {
  UnpackPodResourceDescr<SkeletonResourceDescr>(file, resource->descr);

  Array<u8> data{byte_allocator_};
  UnpackResourceData(file, data);

  const auto* buffer{data.GetData()};
  usize cursor{0};

  memory::CopyMemory(&resource->id, &buffer[cursor], sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&resource->type_id, &buffer[cursor],
                     sizeof(ResourceTypeId));
  cursor += sizeof(ResourceTypeId);

  COMET_ASSERT(resource->type_id == SkeletonResource::kResourceTypeId,
               "Unpacked resource is not a skeleton!");

  memory::CopyMemory(&resource->skeleton.id, &buffer[cursor],
                     sizeof(geometry::SkeletonId));
  cursor += sizeof(geometry::SkeletonId);

  usize joint_count{0};
  memory::CopyMemory(&joint_count, &buffer[cursor], sizeof(joint_count));
  cursor += sizeof(joint_count);

  auto* allocator{ResolveAllocator(byte_allocator_, life_span)};
  resource->skeleton.joints =
      Array<geometry::SkeletonJoint>{allocator, joint_count};

  for (usize i{0}; i < joint_count; ++i) {
    auto& joint{resource->skeleton.joints.EmplaceBack()};

    memory::CopyMemory(&joint.id, &buffer[cursor],
                       sizeof(geometry::SkeletonJointId));
    cursor += sizeof(geometry::SkeletonJointId);

    memory::CopyMemory(&joint.parent_index, &buffer[cursor],
                       sizeof(geometry::SkeletonJointIndex));
    cursor += sizeof(geometry::SkeletonJointIndex);

    memory::CopyMemory(&joint.bind_pose_inv, &buffer[cursor],
                       sizeof(math::Mat4));
    cursor += sizeof(math::Mat4);
  }

  resource->life_span = life_span;
}
}  // namespace resource
}  // namespace comet