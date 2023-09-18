// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model_resource.h"

#include "comet/core/memory/memory.h"

namespace comet {
namespace resource {
const ResourceTypeId ModelResource::kResourceTypeId{COMET_STRING_ID("model")};

uindex ModelHandler::GetMeshSize(const MeshResource& mesh) const {
  constexpr auto kModelIdSize{sizeof(ResourceId)};
  constexpr auto kMeshIdSize{sizeof(ResourceId)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  const auto kVertexCount{mesh.vertices.size()};
  const auto kIndexCount{mesh.indices.size()};

  const auto kVertexCountSize{sizeof(kVertexCount)};
  const auto kIndexCountSize{sizeof(kIndexCount)};

  constexpr auto kVertexSize{sizeof(rendering::Vertex)};
  constexpr auto kIndexSize{sizeof(rendering::Index)};
  constexpr auto kTextureMapSize{sizeof(TextureMap)};

  return kModelIdSize + kMeshIdSize + kMaterialIdSize + kTransformSize +
         kLocalCenterSize + kLocalMaxExtentsSize + kParentMeshIdSize +
         kVertexCountSize + kVertexCount * kVertexSize + kIndexCountSize +
         kIndexCount * kIndexSize;
}

uindex ModelHandler::GetModelSize(const ModelResource& model) const {
  uindex size{sizeof(ResourceId) + sizeof(ResourceTypeId)};

  for (const auto& mesh : model.meshes) {
    size += GetMeshSize(mesh);
  }

  return size;
}

ResourceFile ModelHandler::Pack(const Resource& resource,
                                CompressionMode compression_mode) const {
  const auto& model{static_cast<const ModelResource&>(resource)};
  ResourceFile file{};
  file.resource_id = model.id;
  file.resource_type_id = ModelResource::kResourceTypeId;
  file.compression_mode = compression_mode;

  std::vector<u8> data(GetModelSize(model));
  uindex cursor{0};
  auto* buffer{data.data()};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kMaterialIdSize{sizeof(ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(ResourceId)};
  constexpr auto kVertexCountSize{sizeof(uindex)};
  constexpr auto kIndexCountSize{sizeof(uindex)};
  constexpr auto kVertexSize{sizeof(rendering::Vertex)};
  constexpr auto kIndexSize{sizeof(rendering::Index)};

  CopyMemory(&buffer[cursor], &model.id, kResourceIdSize);
  cursor += kResourceIdSize;

  CopyMemory(&buffer[cursor], &model.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  for (const auto& mesh : model.meshes) {
    CopyMemory(&buffer[cursor], &mesh.resource_id, kResourceIdSize);
    cursor += kResourceIdSize;

    CopyMemory(&buffer[cursor], &mesh.internal_id, kResourceIdSize);
    cursor += kResourceIdSize;

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

std::unique_ptr<Resource> ModelHandler::Unpack(const ResourceFile& file) const {
  ModelResource model{};
  model.descr = UnpackPodResourceDescr<ModelResourceDescr>(file);

  const auto data{UnpackResourceData(file)};
  auto data_size{sizeof(u8) * data.size()};
  const auto* buffer{data.data()};
  uindex cursor{0};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kMaterialIdSize{sizeof(resource::ResourceId)};
  constexpr auto kTransformSize{sizeof(math::Mat4)};
  constexpr auto kLocalCenterSize{sizeof(math::Vec3)};
  constexpr auto kLocalMaxExtentsSize{sizeof(math::Vec3)};
  constexpr auto kParentMeshIdSize{sizeof(resource::ResourceId)};
  constexpr auto kVertexCountSize{sizeof(uindex)};
  constexpr auto kIndexCountSize{sizeof(uindex)};
  constexpr auto kVertexSize{sizeof(rendering::Vertex)};
  constexpr auto kIndexSize{sizeof(rendering::Index)};

  CopyMemory(&model.id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  CopyMemory(&model.type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  while (cursor < data_size) {
    MeshResource mesh{};

    CopyMemory(&mesh.resource_id, &buffer[cursor], kResourceIdSize);
    cursor += kResourceIdSize;

    CopyMemory(&mesh.internal_id, &buffer[cursor], kResourceIdSize);
    cursor += kResourceIdSize;

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

  return std::make_unique<ModelResource>(std::move(model));
}
}  // namespace resource
}  // namespace comet
