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
#include "comet/geometry/geometry_common.h"
#include "comet/math/vector.h"

namespace comet {
namespace resource {
StaticModelResourceHandler::StaticModelResourceHandler(
    const ResourceHandlerDescr& descr)
    : ResourceHandler<StaticModelResource>{descr} {}

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
  usize cursor{0};
  auto* buffer{data.GetData()};

  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
  constexpr auto kMeshCountSize{sizeof(usize)};
  constexpr auto kMeshTypeSize{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  constexpr auto kVertexCountSize{sizeof(usize)};
  constexpr auto kIndexCountSize{sizeof(usize)};
  constexpr auto kVertexSize{sizeof(geometry::SkinnedVertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  memory::CopyMemory(&buffer[cursor], &resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  auto mesh_count{resource.meshes.GetSize()};
  memory::CopyMemory(&buffer[cursor], &mesh_count, kMeshCountSize);
  cursor += kMeshCountSize;

  for (const auto& mesh : resource.meshes) {
    memory::CopyMemory(&buffer[cursor], &mesh.resource_id, kResourceIdSize);
    cursor += kResourceIdSize;

    memory::CopyMemory(&buffer[cursor], &mesh.internal_id, kResourceIdSize);
    cursor += kResourceIdSize;

    memory::CopyMemory(&buffer[cursor], &mesh.type, kMeshTypeSize);
    cursor += kMeshTypeSize;

    memory::CopyMemory(&buffer[cursor], &mesh.material_id, kMaterialIdSize);
    cursor += kMaterialIdSize;

    memory::CopyMemory(&buffer[cursor], &mesh.transform, kTransformSize);
    cursor += kTransformSize;

    memory::CopyMemory(&buffer[cursor], &mesh.local_center, kLocalCenterSize);
    cursor += kLocalCenterSize;

    memory::CopyMemory(&buffer[cursor], &mesh.local_max_extents,
                       kLocalMaxExtentsSize);
    cursor += kLocalMaxExtentsSize;

    memory::CopyMemory(&buffer[cursor], &mesh.parent_id, kParentMeshIdSize);
    cursor += kParentMeshIdSize;

    const auto vertex_count{mesh.vertices.GetSize()};
    const auto index_count{mesh.indices.GetSize()};

    memory::CopyMemory(&buffer[cursor], &vertex_count, kVertexCountSize);

    cursor += kVertexCountSize;
    const auto vertex_total_size{kVertexSize * vertex_count};

    memory::CopyMemory(&buffer[cursor], mesh.vertices.GetData(),
                       vertex_total_size);

    cursor += vertex_total_size;

    memory::CopyMemory(&buffer[cursor], &index_count, kIndexCountSize);

    cursor += kIndexCountSize;
    const auto index_total_size{kIndexSize * index_count};

    memory::CopyMemory(&buffer[cursor], mesh.indices.GetData(),
                       index_total_size);
    cursor += index_total_size;
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

  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
  constexpr auto kMeshCountSize{sizeof(usize)};
  constexpr auto kMeshTypeSize{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  constexpr auto kVertexCountSize{sizeof(usize)};
  constexpr auto kIndexCountSize{sizeof(usize)};
  constexpr auto kVertexSize{sizeof(geometry::SkinnedVertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  memory::CopyMemory(&resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&resource->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  COMET_ASSERT(resource->type_id == StaticModelResource::kResourceTypeId,
               "Model loaded is not static!");

  usize mesh_count;
  memory::CopyMemory(&mesh_count, &buffer[cursor], kMeshCountSize);
  cursor += kMeshCountSize;
  resource->meshes = Array<StaticMeshResource>{
      ResolveAllocator(byte_allocator_, life_span), mesh_count};
  auto& meshes{resource->meshes};

  for (usize i{0}; i < mesh_count; ++i) {
    auto& mesh{meshes.EmplaceBack()};

    memory::CopyMemory(&mesh.resource_id, &buffer[cursor], kResourceIdSize);
    cursor += kResourceIdSize;

    memory::CopyMemory(&mesh.internal_id, &buffer[cursor], kResourceIdSize);
    cursor += kResourceIdSize;

    memory::CopyMemory(&mesh.type, &buffer[cursor], kMeshTypeSize);
    cursor += kMeshTypeSize;

    memory::CopyMemory(&mesh.material_id, &buffer[cursor], kMaterialIdSize);
    cursor += kMaterialIdSize;

    memory::CopyMemory(&mesh.transform, &buffer[cursor], kTransformSize);
    cursor += kTransformSize;

    memory::CopyMemory(&mesh.local_center, &buffer[cursor], kLocalCenterSize);
    cursor += kLocalCenterSize;

    memory::CopyMemory(&mesh.local_max_extents, &buffer[cursor],
                       kLocalMaxExtentsSize);
    cursor += kLocalMaxExtentsSize;

    memory::CopyMemory(&mesh.parent_id, &buffer[cursor], kParentMeshIdSize);
    cursor += kParentMeshIdSize;

    usize vertex_count{0};
    memory::CopyMemory(&vertex_count, &buffer[cursor], kVertexCountSize);
    cursor += kVertexCountSize;

    mesh.vertices = Array<geometry::SkinnedVertex>{
        ResolveAllocator(byte_allocator_, life_span)};
    mesh.vertices.Resize(vertex_count);
    const auto vertex_total_size{kVertexSize * vertex_count};
    memory::CopyMemory(mesh.vertices.GetData(), &buffer[cursor],
                       vertex_total_size);
    cursor += vertex_total_size;

    usize index_count{0};
    memory::CopyMemory(&index_count, &buffer[cursor], kIndexCountSize);
    cursor += kIndexCountSize;

    mesh.indices =
        Array<geometry::Index>{ResolveAllocator(byte_allocator_, life_span)};
    mesh.indices.Resize(index_count);
    const auto index_total_size{kIndexSize * index_count};
    memory::CopyMemory(mesh.indices.GetData(), &buffer[cursor],
                       index_total_size);
    cursor += index_total_size;
  }

  resource->life_span = life_span;
}

SkeletalModelResourceHandler::SkeletalModelResourceHandler(
    const ResourceHandlerDescr& descr)
    : ResourceHandler<SkeletalModelResource>{descr} {}

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
  usize cursor{0};
  auto* buffer{data.GetData()};
  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
  constexpr auto kSkeletonIdSize{sizeof(geometry::SkeletonId)};
  constexpr auto kMeshCountSize{sizeof(usize)};
  constexpr auto kMeshTypeSize{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  constexpr auto kVertexCountSize{sizeof(usize)};
  constexpr auto kIndexCountSize{sizeof(usize)};
  constexpr auto kVertexSize{sizeof(geometry::SkinnedVertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  memory::CopyMemory(&buffer[cursor], &resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.skeleton_id, kSkeletonIdSize);
  cursor += kSkeletonIdSize;

  auto mesh_count{resource.meshes.GetSize()};
  memory::CopyMemory(&buffer[cursor], &mesh_count, kMeshCountSize);
  cursor += kMeshCountSize;

  for (const auto& mesh : resource.meshes) {
    memory::CopyMemory(&buffer[cursor], &mesh.resource_id, kResourceIdSize);
    cursor += kResourceIdSize;

    memory::CopyMemory(&buffer[cursor], &mesh.internal_id, kResourceIdSize);
    cursor += kResourceIdSize;

    memory::CopyMemory(&buffer[cursor], &mesh.type, kMeshTypeSize);
    cursor += kMeshTypeSize;

    memory::CopyMemory(&buffer[cursor], &mesh.material_id, kMaterialIdSize);
    cursor += kMaterialIdSize;

    memory::CopyMemory(&buffer[cursor], &mesh.transform, kTransformSize);
    cursor += kTransformSize;

    memory::CopyMemory(&buffer[cursor], &mesh.local_center, kLocalCenterSize);
    cursor += kLocalCenterSize;

    memory::CopyMemory(&buffer[cursor], &mesh.local_max_extents,
                       kLocalMaxExtentsSize);
    cursor += kLocalMaxExtentsSize;

    memory::CopyMemory(&buffer[cursor], &mesh.parent_id, kParentMeshIdSize);
    cursor += kParentMeshIdSize;

    const auto vertex_count{mesh.vertices.GetSize()};
    const auto index_count{mesh.indices.GetSize()};

    memory::CopyMemory(&buffer[cursor], &vertex_count, kVertexCountSize);

    cursor += kVertexCountSize;
    const auto vertex_total_size{kVertexSize * vertex_count};

    memory::CopyMemory(&buffer[cursor], mesh.vertices.GetData(),
                       vertex_total_size);

    cursor += vertex_total_size;

    memory::CopyMemory(&buffer[cursor], &index_count, kIndexCountSize);

    cursor += kIndexCountSize;
    const auto index_total_size{kIndexSize * index_count};

    memory::CopyMemory(&buffer[cursor], mesh.indices.GetData(),
                       index_total_size);
    cursor += index_total_size;
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

  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
  constexpr auto kSkeletonIdSize{sizeof(geometry::SkeletonId)};
  constexpr auto kMeshCountSize{sizeof(usize)};
  constexpr auto kMeshTypeSize{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  constexpr auto kVertexCountSize{sizeof(usize)};
  constexpr auto kIndexCountSize{sizeof(usize)};
  constexpr auto kVertexSize{sizeof(geometry::SkinnedVertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  memory::CopyMemory(&resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&resource->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&resource->skeleton_id, &buffer[cursor], kSkeletonIdSize);
  cursor += kSkeletonIdSize;

  COMET_ASSERT(resource->type_id == SkeletalModelResource::kResourceTypeId,
               "Model loaded is not a skeleton!");

  usize mesh_count;
  memory::CopyMemory(&mesh_count, &buffer[cursor], kMeshCountSize);
  cursor += kMeshCountSize;
  resource->meshes = Array<SkinnedMeshResource>{
      ResolveAllocator(byte_allocator_, life_span), mesh_count};
  auto& meshes{resource->meshes};

  for (usize i{0}; i < mesh_count; ++i) {
    auto& mesh{meshes.EmplaceBack()};

    memory::CopyMemory(&mesh.resource_id, &buffer[cursor], kResourceIdSize);
    cursor += kResourceIdSize;

    memory::CopyMemory(&mesh.internal_id, &buffer[cursor], kResourceIdSize);
    cursor += kResourceIdSize;

    memory::CopyMemory(&mesh.type, &buffer[cursor], kMeshTypeSize);
    cursor += kMeshTypeSize;

    memory::CopyMemory(&mesh.material_id, &buffer[cursor], kMaterialIdSize);
    cursor += kMaterialIdSize;

    memory::CopyMemory(&mesh.transform, &buffer[cursor], kTransformSize);
    cursor += kTransformSize;

    memory::CopyMemory(&mesh.local_center, &buffer[cursor], kLocalCenterSize);
    cursor += kLocalCenterSize;

    memory::CopyMemory(&mesh.local_max_extents, &buffer[cursor],
                       kLocalMaxExtentsSize);
    cursor += kLocalMaxExtentsSize;

    memory::CopyMemory(&mesh.parent_id, &buffer[cursor], kParentMeshIdSize);
    cursor += kParentMeshIdSize;

    usize vertex_count{0};
    memory::CopyMemory(&vertex_count, &buffer[cursor], kVertexCountSize);
    cursor += kVertexCountSize;

    mesh.vertices = Array<geometry::SkinnedVertex>{
        ResolveAllocator(byte_allocator_, life_span)};
    mesh.vertices.Resize(vertex_count);
    const auto vertex_total_size{kVertexSize * vertex_count};
    memory::CopyMemory(mesh.vertices.GetData(), &buffer[cursor],
                       vertex_total_size);
    cursor += vertex_total_size;

    usize index_count{0};
    memory::CopyMemory(&index_count, &buffer[cursor], kIndexCountSize);
    cursor += kIndexCountSize;

    mesh.indices =
        Array<geometry::Index>{ResolveAllocator(byte_allocator_, life_span)};
    mesh.indices.Resize(index_count);
    const auto index_total_size{kIndexSize * index_count};
    memory::CopyMemory(mesh.indices.GetData(), &buffer[cursor],
                       index_total_size);
    cursor += index_total_size;
  }

  resource->life_span = life_span;
}

SkeletonResourceHandler::SkeletonResourceHandler(
    const ResourceHandlerDescr& descr)
    : ResourceHandler<SkeletonResource>{descr} {}

ResourceFile SkeletonResourceHandler::Pack(const SkeletonResource& resource,
                                           CompressionMode compression_mode) {
  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = SkeletalModelResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{byte_allocator_};
  file.data = Array<u8>{byte_allocator_};

  Array<u8> data{byte_allocator_};
  data.Resize(GetSkeletonSize(resource));
  usize cursor{0};
  auto* buffer{data.GetData()};

  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
  constexpr auto kSkeletonIdSize{sizeof(geometry::SkeletonId)};
  constexpr auto kJointCountSize{sizeof(usize)};
  constexpr auto kSkeletonJointIdSize{sizeof(geometry::SkeletonJointId)};
  constexpr auto kSkeletonJointIndexSize{sizeof(geometry::SkeletonJointIndex)};
  constexpr auto kSkeletonJointBindPoseInvSize{sizeof(math::Mat4)};

  memory::CopyMemory(&buffer[cursor], &resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.skeleton.id, kSkeletonIdSize);
  cursor += kSkeletonIdSize;

  auto& skeleton{resource.skeleton};
  auto joint_count{skeleton.joints.GetSize()};
  memory::CopyMemory(&buffer[cursor], &joint_count, kJointCountSize);
  cursor += kJointCountSize;

  for (const auto& joint : skeleton.joints) {
    memory::CopyMemory(&buffer[cursor], &joint.id, kSkeletonJointIdSize);
    cursor += kSkeletonJointIdSize;

    memory::CopyMemory(&buffer[cursor], &joint.parent_index,
                       kSkeletonJointIndexSize);
    cursor += kSkeletonJointIndexSize;

    memory::CopyMemory(&buffer[cursor], &joint.bind_pose_inv,
                       kSkeletonJointBindPoseInvSize);
    cursor += kSkeletonJointBindPoseInvSize;
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

  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
  constexpr auto kSkeletonIdSize{sizeof(geometry::SkeletonId)};
  constexpr auto kJointCountSize{sizeof(usize)};
  constexpr auto kSkeletonJointIdSize{sizeof(geometry::SkeletonJointId)};
  constexpr auto kSkeletonJointIndexSize{sizeof(geometry::SkeletonJointIndex)};
  constexpr auto kSkeletonJointBindPoseInvSize{sizeof(math::Mat4)};

  memory::CopyMemory(&resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&resource->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  auto& skeleton{resource->skeleton};
  memory::CopyMemory(&resource->skeleton.id, &buffer[cursor], kSkeletonIdSize);
  cursor += kSkeletonIdSize;

  usize joint_count;
  memory::CopyMemory(&joint_count, &buffer[cursor], kJointCountSize);
  cursor += kJointCountSize;
  skeleton.joints = Array<geometry::SkeletonJoint>{
      ResolveAllocator(byte_allocator_, life_span), joint_count};
  auto& joints{skeleton.joints};

  for (usize i{0}; i < joint_count; ++i) {
    auto& joint{joints.EmplaceBack()};

    memory::CopyMemory(&joint.id, &buffer[cursor], kSkeletonJointIdSize);
    cursor += kSkeletonJointIdSize;

    memory::CopyMemory(&joint.parent_index, &buffer[cursor],
                       kSkeletonJointIndexSize);
    cursor += kSkeletonJointIndexSize;

    memory::CopyMemory(&joint.bind_pose_inv, &buffer[cursor],
                       kSkeletonJointBindPoseInvSize);
    cursor += kSkeletonJointBindPoseInvSize;
  }

  resource->life_span = life_span;
}
}  // namespace resource
}  // namespace comet
