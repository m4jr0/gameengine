// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "model_resource.h"

#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"

namespace comet {
namespace resource {
const ResourceTypeId StaticModelResource::kResourceTypeId{
    COMET_STRING_ID("static_model")};

const ResourceTypeId SkeletalModelResource::kResourceTypeId{
    COMET_STRING_ID("skeletal_model")};

const ResourceTypeId SkeletonResource::kResourceTypeId{
    COMET_STRING_ID("skeleton")};

StaticModelHandler::StaticModelHandler(
    memory::Allocator* loading_resources_allocator,
    memory::Allocator* loading_resource_allocator)
    : ResourceHandler{sizeof(StaticModelResource), loading_resources_allocator,
                      loading_resource_allocator} {}

usize StaticModelHandler::GetMeshSize(const StaticMeshResource& mesh) const {
  constexpr auto kModelIdSize{sizeof(ResourceId)};
  constexpr auto kMeshIdSize{sizeof(ResourceId)};
  constexpr auto kMeshType{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  const auto kVertexCount{mesh.vertices.GetSize()};
  const auto kIndexCount{mesh.indices.GetSize()};

  const auto kVertexCountSize{sizeof(kVertexCount)};
  const auto kIndexCountSize{sizeof(kIndexCount)};

  constexpr auto kVertexSize{sizeof(geometry::SkinnedVertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  return kModelIdSize + kMeshIdSize + kMeshType + kMaterialIdSize +
         kTransformSize + kLocalCenterSize + kLocalMaxExtentsSize +
         kParentMeshIdSize + kVertexCountSize + kVertexCount * kVertexSize +
         kIndexCountSize + kIndexCount * kIndexSize;
}

usize StaticModelHandler::GetModelSize(const StaticModelResource& model) const {
  usize size{sizeof(ResourceId) + sizeof(ResourceTypeId) + sizeof(usize)};

  for (const auto& mesh : model.meshes) {
    size += GetMeshSize(mesh);
  }

  return size;
}

ResourceFile StaticModelHandler::Pack(memory::Allocator& allocator,
                                      const Resource& resource,
                                      CompressionMode compression_mode) const {
  const auto& model{static_cast<const StaticModelResource&>(resource)};
  ResourceFile file{};
  file.resource_id = model.id;
  file.resource_type_id = StaticModelResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{&allocator};
  file.data = Array<u8>{&allocator};

  Array<u8> data{&allocator};
  data.Resize(GetModelSize(model));
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

  memory::CopyMemory(&buffer[cursor], &model.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &model.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  auto mesh_count{model.meshes.GetSize()};
  memory::CopyMemory(&buffer[cursor], &mesh_count, kMeshCountSize);
  cursor += kMeshCountSize;

  for (const auto& mesh : model.meshes) {
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

  PackPodResourceDescr(model.descr, file);
  PackResourceData(data, file);
  return file;
}

Resource* StaticModelHandler::Unpack(memory::Allocator& allocator,
                                     const ResourceFile& file) {
  auto* model{
      resource_allocator_.AllocateOneAndPopulate<StaticModelResource>()};
  UnpackPodResourceDescr<StaticModelResourceDescr>(file, model->descr);

  Array<u8> data{&allocator};
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

  memory::CopyMemory(&model->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&model->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  COMET_ASSERT(model->type_id == StaticModelResource::kResourceTypeId,
               "Model loaded is not static!");

  usize mesh_count;
  memory::CopyMemory(&mesh_count, &buffer[cursor], kMeshCountSize);
  cursor += kMeshCountSize;
  model->meshes = Array<StaticMeshResource>{&allocator, mesh_count};
  auto& meshes{model->meshes};

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

    mesh.vertices = Array<geometry::SkinnedVertex>{&allocator};
    mesh.vertices.Resize(vertex_count);
    const auto vertex_total_size{kVertexSize * vertex_count};
    memory::CopyMemory(mesh.vertices.GetData(), &buffer[cursor],
                       vertex_total_size);
    cursor += vertex_total_size;

    usize index_count{0};
    memory::CopyMemory(&index_count, &buffer[cursor], kIndexCountSize);
    cursor += kIndexCountSize;

    mesh.indices = Array<geometry::Index>{&allocator};
    mesh.indices.Resize(index_count);
    const auto index_total_size{kIndexSize * index_count};
    memory::CopyMemory(mesh.indices.GetData(), &buffer[cursor],
                       index_total_size);
    cursor += index_total_size;
  }

  return model;
}

SkeletalModelHandler::SkeletalModelHandler(
    memory::Allocator* loading_resources_allocator,
    memory::Allocator* loading_resource_allocator)
    : ResourceHandler{sizeof(SkeletalModelResource),
                      loading_resources_allocator, loading_resource_allocator} {
}

usize SkeletalModelHandler::GetMeshSize(const SkinnedMeshResource& mesh) const {
  constexpr auto kModelIdSize{sizeof(ResourceId)};
  constexpr auto kMeshIdSize{sizeof(ResourceId)};
  constexpr auto kMeshType{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  const auto kVertexCount{mesh.vertices.GetSize()};
  const auto kIndexCount{mesh.indices.GetSize()};

  const auto kVertexCountSize{sizeof(kVertexCount)};
  const auto kIndexCountSize{sizeof(kIndexCount)};

  constexpr auto kVertexSize{sizeof(geometry::SkinnedVertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  return kModelIdSize + kMeshIdSize + kMeshType + kMaterialIdSize +
         kTransformSize + kLocalCenterSize + kLocalMaxExtentsSize +
         kParentMeshIdSize + kVertexCountSize + kVertexCount * kVertexSize +
         kIndexCountSize + kIndexCount * kIndexSize;
}

usize SkeletalModelHandler::GetModelSize(
    const SkeletalModelResource& model) const {
  usize size{sizeof(ResourceId) + sizeof(ResourceTypeId) +
             sizeof(geometry::SkeletonId) + sizeof(usize)};

  for (const auto& mesh : model.meshes) {
    size += GetMeshSize(mesh);
  }

  return size;
}

ResourceFile SkeletalModelHandler::Pack(
    memory::Allocator& allocator, const Resource& resource,
    CompressionMode compression_mode) const {
  const auto& model{static_cast<const SkeletalModelResource&>(resource)};
  ResourceFile file{};
  file.resource_id = model.id;
  file.resource_type_id = SkeletalModelResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{&allocator};
  file.data = Array<u8>{&allocator};

  Array<u8> data{&allocator};
  data.Resize(GetModelSize(model));
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

  memory::CopyMemory(&buffer[cursor], &model.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &model.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&buffer[cursor], &model.skeleton_id, kSkeletonIdSize);
  cursor += kSkeletonIdSize;

  auto mesh_count{model.meshes.GetSize()};
  memory::CopyMemory(&buffer[cursor], &mesh_count, kMeshCountSize);
  cursor += kMeshCountSize;

  for (const auto& mesh : model.meshes) {
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

  PackPodResourceDescr(model.descr, file);
  PackResourceData(data, file);
  return file;
}

Resource* SkeletalModelHandler::Unpack(memory::Allocator& allocator,
                                       const ResourceFile& file) {
  auto* model{
      resource_allocator_.AllocateOneAndPopulate<SkeletalModelResource>()};
  UnpackPodResourceDescr<SkeletalModelResourceDescr>(file, model->descr);

  Array<u8> data{&allocator};
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

  memory::CopyMemory(&model->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&model->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&model->skeleton_id, &buffer[cursor], kSkeletonIdSize);
  cursor += kSkeletonIdSize;

  COMET_ASSERT(model->type_id == SkeletalModelResource::kResourceTypeId,
               "Model loaded is not a skeleton!");

  usize mesh_count;
  memory::CopyMemory(&mesh_count, &buffer[cursor], kMeshCountSize);
  cursor += kMeshCountSize;
  model->meshes = Array<SkinnedMeshResource>{&allocator, mesh_count};
  auto& meshes{model->meshes};

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

    mesh.vertices = Array<geometry::SkinnedVertex>{&allocator};
    mesh.vertices.Resize(vertex_count);
    const auto vertex_total_size{kVertexSize * vertex_count};
    memory::CopyMemory(mesh.vertices.GetData(), &buffer[cursor],
                       vertex_total_size);
    cursor += vertex_total_size;

    usize index_count{0};
    memory::CopyMemory(&index_count, &buffer[cursor], kIndexCountSize);
    cursor += kIndexCountSize;

    mesh.indices = Array<geometry::Index>{&allocator};
    mesh.indices.Resize(index_count);
    const auto index_total_size{kIndexSize * index_count};
    memory::CopyMemory(mesh.indices.GetData(), &buffer[cursor],
                       index_total_size);
    cursor += index_total_size;
  }

  return model;
}

SkeletonHandler::SkeletonHandler(memory::Allocator* loading_resources_allocator,
                                 memory::Allocator* loading_resource_allocator)
    : ResourceHandler{sizeof(SkeletalModelResource),
                      loading_resources_allocator, loading_resource_allocator} {
}

usize SkeletonHandler::GetSkeletonJointSize() const {
  return sizeof(geometry::SkeletonJointId) + sizeof(math::Mat4) +
         sizeof(geometry::SkeletonJointIndex);
}

usize SkeletonHandler::GetSkeletonSize(const SkeletonResource& skeleton) const {
  usize size{sizeof(ResourceId) + sizeof(ResourceTypeId) +
             sizeof(geometry::SkeletonId) + sizeof(usize)};

  auto joint_count{skeleton.skeleton.joints.GetSize()};

  for (usize i{0}; i < joint_count; ++i) {
    size += GetSkeletonJointSize();
  }

  return size;
}

ResourceFile SkeletonHandler::Pack(memory::Allocator& allocator,
                                   const Resource& resource,
                                   CompressionMode compression_mode) const {
  const auto& skeleton{static_cast<const SkeletonResource&>(resource)};
  ResourceFile file{};
  file.resource_id = skeleton.id;
  file.resource_type_id = SkeletalModelResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{&allocator};
  file.data = Array<u8>{&allocator};

  Array<u8> data{&allocator};
  data.Resize(GetSkeletonSize(skeleton));
  usize cursor{0};
  auto* buffer{data.GetData()};

  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
  constexpr auto kSkeletonIdSize{sizeof(geometry::SkeletonId)};
  constexpr auto kJointCountSize{sizeof(usize)};
  constexpr auto kSkeletonJointIdSize{sizeof(geometry::SkeletonJointId)};
  constexpr auto kSkeletonJointIndexSize{sizeof(geometry::SkeletonJointIndex)};
  constexpr auto kSkeletonJointBindPoseInvSize{sizeof(math::Mat4)};

  memory::CopyMemory(&buffer[cursor], &skeleton.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &skeleton.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&buffer[cursor], &skeleton.skeleton.id, kSkeletonIdSize);
  cursor += kSkeletonIdSize;

  auto joint_count{skeleton.skeleton.joints.GetSize()};
  memory::CopyMemory(&buffer[cursor], &joint_count, kJointCountSize);
  cursor += kJointCountSize;

  for (const auto& joint : skeleton.skeleton.joints) {
    memory::CopyMemory(&buffer[cursor], &joint.id, kSkeletonJointIdSize);
    cursor += kSkeletonJointIdSize;

    memory::CopyMemory(&buffer[cursor], &joint.parent_index,
                       kSkeletonJointIndexSize);
    cursor += kSkeletonJointIndexSize;

    memory::CopyMemory(&buffer[cursor], &joint.bind_pose_inv,
                       kSkeletonJointBindPoseInvSize);
    cursor += kSkeletonJointBindPoseInvSize;
  }

  PackPodResourceDescr(skeleton.descr, file);
  PackResourceData(data, file);
  return file;
}

Resource* SkeletonHandler::Unpack(memory::Allocator& allocator,
                                  const ResourceFile& file) {
  auto* skeleton{
      resource_allocator_.AllocateOneAndPopulate<SkeletonResource>()};
  UnpackPodResourceDescr<SkeletonResourceDescr>(file, skeleton->descr);

  Array<u8> data{&allocator};
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

  memory::CopyMemory(&skeleton->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&skeleton->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&skeleton->skeleton.id, &buffer[cursor], kSkeletonIdSize);
  cursor += kSkeletonIdSize;

  usize joint_count;
  memory::CopyMemory(&joint_count, &buffer[cursor], kJointCountSize);
  cursor += kJointCountSize;
  skeleton->skeleton.joints =
      Array<geometry::SkeletonJoint>{&allocator, joint_count};
  auto& joints{skeleton->skeleton.joints};

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

  return skeleton;
}
}  // namespace resource
}  // namespace comet
