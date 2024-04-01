// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model_resource.h"

#include "comet/core/memory/memory.h"

namespace comet {
namespace resource {
const ResourceTypeId StaticModelResource::kResourceTypeId{
    COMET_STRING_ID("static_model")};

const ResourceTypeId SkeletalModelResource::kResourceTypeId{
    COMET_STRING_ID("skeletal_model")};

usize StaticModelHandler::GetMeshSize(const StaticMeshResource& mesh) const {
  constexpr auto kModelIdSize{sizeof(ResourceId)};
  constexpr auto kMeshIdSize{sizeof(ResourceId)};
  constexpr auto kMeshType{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  const auto kVertexCount{mesh.vertices.size()};
  const auto kIndexCount{mesh.indices.size()};

  const auto kVertexCountSize{sizeof(kVertexCount)};
  const auto kIndexCountSize{sizeof(kIndexCount)};

  constexpr auto kVertexSize{sizeof(geometry::Vertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  return kModelIdSize + kMeshIdSize + kMeshType + kMaterialIdSize +
         kTransformSize + kLocalCenterSize + kLocalMaxExtentsSize +
         kParentMeshIdSize + kVertexCountSize + kVertexCount * kVertexSize +
         kIndexCountSize + kIndexCount * kIndexSize;
}

usize StaticModelHandler::GetModelSize(const StaticModelResource& model) const {
  usize size{sizeof(ResourceId) + sizeof(ResourceTypeId)};

  for (const auto& mesh : model.meshes) {
    size += GetMeshSize(mesh);
  }

  return size;
}

ResourceFile StaticModelHandler::Pack(const Resource& resource,
                                      CompressionMode compression_mode) const {
  const auto& model{static_cast<const StaticModelResource&>(resource)};
  ResourceFile file{};
  file.resource_id = model.id;
  file.resource_type_id = StaticModelResource::kResourceTypeId;
  file.compression_mode = compression_mode;

  std::vector<u8> data(GetModelSize(model));
  usize cursor{0};
  auto* buffer{data.data()};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kMeshTypeSize{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  constexpr auto kVertexCountSize{sizeof(usize)};
  constexpr auto kIndexCountSize{sizeof(usize)};
  constexpr auto kVertexSize{sizeof(geometry::Vertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  memory::CopyMemory(&buffer[cursor], &model.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &model.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

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

    const auto vertex_count{mesh.vertices.size()};
    const auto index_count{mesh.indices.size()};

    memory::CopyMemory(&buffer[cursor], &vertex_count, kVertexCountSize);

    cursor += kVertexCountSize;
    const auto vertex_total_size{kVertexSize * vertex_count};

    memory::CopyMemory(&buffer[cursor], mesh.vertices.data(),
                       vertex_total_size);

    cursor += vertex_total_size;

    memory::CopyMemory(&buffer[cursor], &index_count, kIndexCountSize);

    cursor += kIndexCountSize;
    const auto index_total_size{kIndexSize * index_count};

    memory::CopyMemory(&buffer[cursor], mesh.indices.data(), index_total_size);
    cursor += index_total_size;
  }

  PackPodResourceDescr(model.descr, file);
  PackResourceData(data, file);
  return file;
}

std::unique_ptr<Resource> StaticModelHandler::Unpack(
    const ResourceFile& file) const {
  StaticModelResource model{};
  model.descr = UnpackPodResourceDescr<StaticModelResourceDescr>(file);

  const auto data{UnpackResourceData(file)};
  auto data_size{sizeof(u8) * data.size()};
  const auto* buffer{data.data()};
  usize cursor{0};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kMeshTypeSize{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(resource::ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(resource::ResourceId)};
  constexpr auto kVertexCountSize{sizeof(usize)};
  constexpr auto kIndexCountSize{sizeof(usize)};
  constexpr auto kVertexSize{sizeof(geometry::Vertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  memory::CopyMemory(&model.id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&model.type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  COMET_ASSERT(model.type_id == StaticModelResource::kResourceTypeId,
               "Model loaded is not static!");

  while (cursor < data_size) {
    StaticMeshResource mesh{};

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

    mesh.vertices.resize(vertex_count);
    const auto vertex_total_size{kVertexSize * vertex_count};
    memory::CopyMemory(mesh.vertices.data(), &buffer[cursor],
                       vertex_total_size);
    cursor += vertex_total_size;

    usize index_count{0};
    memory::CopyMemory(&index_count, &buffer[cursor], kIndexCountSize);
    cursor += kIndexCountSize;

    mesh.indices.resize(index_count);
    const auto index_total_size{kIndexSize * index_count};
    memory::CopyMemory(mesh.indices.data(), &buffer[cursor], index_total_size);
    cursor += index_total_size;

    model.meshes.push_back(std::move(mesh));
  }

  return std::make_unique<StaticModelResource>(std::move(model));
}

usize SkinnedModelHandler::GetMeshSize(const SkinnedMeshResource& mesh) const {
  constexpr auto kModelIdSize{sizeof(ResourceId)};
  constexpr auto kMeshIdSize{sizeof(ResourceId)};
  constexpr auto kMeshType{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  const auto kVertexCount{mesh.vertices.size()};
  const auto kIndexCount{mesh.indices.size()};

  const auto kVertexCountSize{sizeof(kVertexCount)};
  const auto kIndexCountSize{sizeof(kIndexCount)};

  constexpr auto kVertexSize{sizeof(geometry::SkinnedVertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  return kModelIdSize + kMeshIdSize + kMeshType + kMaterialIdSize +
         kTransformSize + kLocalCenterSize + kLocalMaxExtentsSize +
         kParentMeshIdSize + kVertexCountSize + kVertexCount * kVertexSize +
         kIndexCountSize + kIndexCount * kIndexSize;
}

usize SkinnedModelHandler::GetModelSize(
    const SkeletalModelResource& model) const {
  usize size{sizeof(ResourceId) + sizeof(ResourceTypeId)};

  for (const auto& mesh : model.meshes) {
    size += GetMeshSize(mesh);
  }

  return size;
}

ResourceFile SkinnedModelHandler::Pack(const Resource& resource,
                                       CompressionMode compression_mode) const {
  const auto& model{static_cast<const SkeletalModelResource&>(resource)};
  ResourceFile file{};
  file.resource_id = model.id;
  file.resource_type_id = SkeletalModelResource::kResourceTypeId;
  file.compression_mode = compression_mode;

  std::vector<u8> data(GetModelSize(model));
  usize cursor{0};
  auto* buffer{data.data()};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
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

    const auto vertex_count{mesh.vertices.size()};
    const auto index_count{mesh.indices.size()};

    memory::CopyMemory(&buffer[cursor], &vertex_count, kVertexCountSize);

    cursor += kVertexCountSize;
    const auto vertex_total_size{kVertexSize * vertex_count};

    memory::CopyMemory(&buffer[cursor], mesh.vertices.data(),
                       vertex_total_size);

    cursor += vertex_total_size;

    memory::CopyMemory(&buffer[cursor], &index_count, kIndexCountSize);

    cursor += kIndexCountSize;
    const auto index_total_size{kIndexSize * index_count};

    memory::CopyMemory(&buffer[cursor], mesh.indices.data(), index_total_size);
    cursor += index_total_size;
  }

  PackPodResourceDescr(model.descr, file);
  PackResourceData(data, file);
  return file;
}

std::unique_ptr<Resource> SkinnedModelHandler::Unpack(
    const ResourceFile& file) const {
  SkeletalModelResource model{};
  model.descr = UnpackPodResourceDescr<SkeletalModelResourceDescr>(file);

  const auto data{UnpackResourceData(file)};
  auto data_size{sizeof(u8) * data.size()};
  const auto* buffer{data.data()};
  usize cursor{0};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kMeshTypeSize{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(resource::ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(resource::ResourceId)};
  constexpr auto kVertexCountSize{sizeof(usize)};
  constexpr auto kIndexCountSize{sizeof(usize)};
  constexpr auto kVertexSize{sizeof(geometry::SkinnedVertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  memory::CopyMemory(&model.id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&model.type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  COMET_ASSERT(model.type_id == SkeletalModelResource::kResourceTypeId,
               "Model loaded is not a skeleton!");

  while (cursor < data_size) {
    SkinnedMeshResource mesh{};

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

    mesh.vertices.resize(vertex_count);
    const auto vertex_total_size{kVertexSize * vertex_count};
    memory::CopyMemory(mesh.vertices.data(), &buffer[cursor],
                       vertex_total_size);
    cursor += vertex_total_size;

    usize index_count{0};
    memory::CopyMemory(&index_count, &buffer[cursor], kIndexCountSize);
    cursor += kIndexCountSize;

    mesh.indices.resize(index_count);
    const auto index_total_size{kIndexSize * index_count};
    memory::CopyMemory(mesh.indices.data(), &buffer[cursor], index_total_size);
    cursor += index_total_size;

    model.meshes.push_back(std::move(mesh));
  }

  return std::make_unique<SkeletalModelResource>(std::move(model));
}
}  // namespace resource
}  // namespace comet
