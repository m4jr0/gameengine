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

uindex StaticModelHandler::GetMeshSize(const StaticMeshResource& mesh) const {
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

uindex StaticModelHandler::GetModelSize(
    const StaticModelResource& model) const {
  uindex size{sizeof(ResourceId) + sizeof(ResourceTypeId)};

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
  uindex cursor{0};
  auto* buffer{data.data()};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kMeshTypeSize{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  constexpr auto kVertexCountSize{sizeof(uindex)};
  constexpr auto kIndexCountSize{sizeof(uindex)};
  constexpr auto kVertexSize{sizeof(geometry::Vertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  CopyMemory(&buffer[cursor], &model.id, kResourceIdSize);
  cursor += kResourceIdSize;

  CopyMemory(&buffer[cursor], &model.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  for (const auto& mesh : model.meshes) {
    CopyMemory(&buffer[cursor], &mesh.resource_id, kResourceIdSize);
    cursor += kResourceIdSize;

    CopyMemory(&buffer[cursor], &mesh.internal_id, kResourceIdSize);
    cursor += kResourceIdSize;

    CopyMemory(&buffer[cursor], &mesh.type, kMeshTypeSize);
    cursor += kMeshTypeSize;

    CopyMemory(&buffer[cursor], &mesh.material_id, kMaterialIdSize);
    cursor += kMaterialIdSize;

    CopyMemory(&buffer[cursor], &mesh.transform, kTransformSize);
    cursor += kTransformSize;

    CopyMemory(&buffer[cursor], &mesh.local_center, kLocalCenterSize);
    cursor += kLocalCenterSize;

    CopyMemory(&buffer[cursor], &mesh.local_max_extents, kLocalMaxExtentsSize);
    cursor += kLocalMaxExtentsSize;

    CopyMemory(&buffer[cursor], &mesh.parent_id, kParentMeshIdSize);
    cursor += kParentMeshIdSize;

    const auto vertex_count{mesh.vertices.size()};
    const auto index_count{mesh.indices.size()};

    CopyMemory(&buffer[cursor], &vertex_count, kVertexCountSize);

    cursor += kVertexCountSize;
    const auto vertex_total_size{kVertexSize * vertex_count};

    CopyMemory(&buffer[cursor], mesh.vertices.data(), vertex_total_size);

    cursor += vertex_total_size;

    CopyMemory(&buffer[cursor], &index_count, kIndexCountSize);

    cursor += kIndexCountSize;
    const auto index_total_size{kIndexSize * index_count};

    CopyMemory(&buffer[cursor], mesh.indices.data(), index_total_size);
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
  uindex cursor{0};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kMeshTypeSize{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(resource::ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(resource::ResourceId)};
  constexpr auto kVertexCountSize{sizeof(uindex)};
  constexpr auto kIndexCountSize{sizeof(uindex)};
  constexpr auto kVertexSize{sizeof(geometry::Vertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  CopyMemory(&model.id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  CopyMemory(&model.type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  COMET_ASSERT(model.type_id == StaticModelResource::kResourceTypeId,
               "Model loaded is not static!");

  while (cursor < data_size) {
    StaticMeshResource mesh{};

    CopyMemory(&mesh.resource_id, &buffer[cursor], kResourceIdSize);
    cursor += kResourceIdSize;

    CopyMemory(&mesh.internal_id, &buffer[cursor], kResourceIdSize);
    cursor += kResourceIdSize;

    CopyMemory(&mesh.type, &buffer[cursor], kMeshTypeSize);
    cursor += kMeshTypeSize;

    CopyMemory(&mesh.material_id, &buffer[cursor], kMaterialIdSize);
    cursor += kMaterialIdSize;

    CopyMemory(&mesh.transform, &buffer[cursor], kTransformSize);
    cursor += kTransformSize;

    CopyMemory(&mesh.local_center, &buffer[cursor], kLocalCenterSize);
    cursor += kLocalCenterSize;

    CopyMemory(&mesh.local_max_extents, &buffer[cursor], kLocalMaxExtentsSize);
    cursor += kLocalMaxExtentsSize;

    CopyMemory(&mesh.parent_id, &buffer[cursor], kParentMeshIdSize);
    cursor += kParentMeshIdSize;

    uindex vertex_count{0};
    CopyMemory(&vertex_count, &buffer[cursor], kVertexCountSize);
    cursor += kVertexCountSize;

    mesh.vertices.resize(vertex_count);
    const auto vertex_total_size{kVertexSize * vertex_count};
    CopyMemory(mesh.vertices.data(), &buffer[cursor], vertex_total_size);
    cursor += vertex_total_size;

    uindex index_count{0};
    CopyMemory(&index_count, &buffer[cursor], kIndexCountSize);
    cursor += kIndexCountSize;

    mesh.indices.resize(index_count);
    const auto index_total_size{kIndexSize * index_count};
    CopyMemory(mesh.indices.data(), &buffer[cursor], index_total_size);
    cursor += index_total_size;

    model.meshes.push_back(std::move(mesh));
  }

  return std::make_unique<StaticModelResource>(std::move(model));
}

uindex SkinnedModelHandler::GetMeshSize(const SkinnedMeshResource& mesh) const {
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

uindex SkinnedModelHandler::GetModelSize(
    const SkeletalModelResource& model) const {
  uindex size{sizeof(ResourceId) + sizeof(ResourceTypeId)};

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
  uindex cursor{0};
  auto* buffer{data.data()};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kMeshTypeSize{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  constexpr auto kVertexCountSize{sizeof(uindex)};
  constexpr auto kIndexCountSize{sizeof(uindex)};
  constexpr auto kVertexSize{sizeof(geometry::SkinnedVertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  CopyMemory(&buffer[cursor], &model.id, kResourceIdSize);
  cursor += kResourceIdSize;

  CopyMemory(&buffer[cursor], &model.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  for (const auto& mesh : model.meshes) {
    CopyMemory(&buffer[cursor], &mesh.resource_id, kResourceIdSize);
    cursor += kResourceIdSize;

    CopyMemory(&buffer[cursor], &mesh.internal_id, kResourceIdSize);
    cursor += kResourceIdSize;

    CopyMemory(&buffer[cursor], &mesh.type, kMeshTypeSize);
    cursor += kMeshTypeSize;

    CopyMemory(&buffer[cursor], &mesh.material_id, kMaterialIdSize);
    cursor += kMaterialIdSize;

    CopyMemory(&buffer[cursor], &mesh.transform, kTransformSize);
    cursor += kTransformSize;

    CopyMemory(&buffer[cursor], &mesh.local_center, kLocalCenterSize);
    cursor += kLocalCenterSize;

    CopyMemory(&buffer[cursor], &mesh.local_max_extents, kLocalMaxExtentsSize);
    cursor += kLocalMaxExtentsSize;

    CopyMemory(&buffer[cursor], &mesh.parent_id, kParentMeshIdSize);
    cursor += kParentMeshIdSize;

    const auto vertex_count{mesh.vertices.size()};
    const auto index_count{mesh.indices.size()};

    CopyMemory(&buffer[cursor], &vertex_count, kVertexCountSize);

    cursor += kVertexCountSize;
    const auto vertex_total_size{kVertexSize * vertex_count};

    CopyMemory(&buffer[cursor], mesh.vertices.data(), vertex_total_size);

    cursor += vertex_total_size;

    CopyMemory(&buffer[cursor], &index_count, kIndexCountSize);

    cursor += kIndexCountSize;
    const auto index_total_size{kIndexSize * index_count};

    CopyMemory(&buffer[cursor], mesh.indices.data(), index_total_size);
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
  uindex cursor{0};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kMeshTypeSize{sizeof(geometry::MeshType)};
  constexpr auto kMaterialIdSize{sizeof(resource::ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(resource::ResourceId)};
  constexpr auto kVertexCountSize{sizeof(uindex)};
  constexpr auto kIndexCountSize{sizeof(uindex)};
  constexpr auto kVertexSize{sizeof(geometry::SkinnedVertex)};
  constexpr auto kIndexSize{sizeof(geometry::Index)};

  CopyMemory(&model.id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  CopyMemory(&model.type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  COMET_ASSERT(model.type_id == SkeletalModelResource::kResourceTypeId,
               "Model loaded is not a skeleton!");

  while (cursor < data_size) {
    SkinnedMeshResource mesh{};

    CopyMemory(&mesh.resource_id, &buffer[cursor], kResourceIdSize);
    cursor += kResourceIdSize;

    CopyMemory(&mesh.internal_id, &buffer[cursor], kResourceIdSize);
    cursor += kResourceIdSize;

    CopyMemory(&mesh.type, &buffer[cursor], kMeshTypeSize);
    cursor += kMeshTypeSize;

    CopyMemory(&mesh.material_id, &buffer[cursor], kMaterialIdSize);
    cursor += kMaterialIdSize;

    CopyMemory(&mesh.transform, &buffer[cursor], kTransformSize);
    cursor += kTransformSize;

    CopyMemory(&mesh.local_center, &buffer[cursor], kLocalCenterSize);
    cursor += kLocalCenterSize;

    CopyMemory(&mesh.local_max_extents, &buffer[cursor], kLocalMaxExtentsSize);
    cursor += kLocalMaxExtentsSize;

    CopyMemory(&mesh.parent_id, &buffer[cursor], kParentMeshIdSize);
    cursor += kParentMeshIdSize;

    uindex vertex_count{0};
    CopyMemory(&vertex_count, &buffer[cursor], kVertexCountSize);
    cursor += kVertexCountSize;

    mesh.vertices.resize(vertex_count);
    const auto vertex_total_size{kVertexSize * vertex_count};
    CopyMemory(mesh.vertices.data(), &buffer[cursor], vertex_total_size);
    cursor += vertex_total_size;

    uindex index_count{0};
    CopyMemory(&index_count, &buffer[cursor], kIndexCountSize);
    cursor += kIndexCountSize;

    mesh.indices.resize(index_count);
    const auto index_total_size{kIndexSize * index_count};
    CopyMemory(mesh.indices.data(), &buffer[cursor], index_total_size);
    cursor += index_total_size;

    model.meshes.push_back(std::move(mesh));
  }

  return std::make_unique<SkeletalModelResource>(std::move(model));
}
}  // namespace resource
}  // namespace comet
