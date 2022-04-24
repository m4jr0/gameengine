// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model_resource.h"

namespace comet {
namespace resource {
namespace model {
const ResourceTypeId ModelResource::kResourceTypeId{
    GenerateResourceTypeId("model")};

uindex ModelHandler::GetMeshSize(const MeshResource& mesh) const {
  constexpr uindex kModelIdSize{sizeof(stringid::StringId)};
  const uindex kVertexCount{mesh.vertices.size()};
  const uindex kIndexCount{mesh.indices.size()};
  const uindex kTextureTuplesCount{mesh.textures.size()};

  const auto kVertexCountSize{sizeof(kVertexCount)};
  const auto kIndexCountSize{sizeof(kIndexCount)};
  const auto kTextureTupleCountSize{sizeof(kTextureTuplesCount)};

  constexpr auto kVertexSize{sizeof(Vertex)};
  constexpr auto kIndexSize{sizeof(Index)};
  constexpr auto kTextureTupleSize{sizeof(TextureTuple)};

  return kModelIdSize + kVertexCountSize + kVertexCount * kVertexSize +
         kIndexCountSize + kIndexCount * kIndexSize + kTextureTupleCountSize +
         kTextureTuplesCount * kTextureTupleSize;
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
  file.resource_type_id = ModelResource::kResourceTypeId;
  file.compression_mode = compression_mode;

  std::vector<char> data(GetModelSize(model));
  uindex cursor{0};
  auto* buffer{data.data()};
  constexpr auto kResourceIdSize{sizeof(stringid::StringId)};
  constexpr auto kResourceTypeIdSize{sizeof(stringid::StringId)};
  constexpr auto kVertexCountSize{sizeof(uindex)};
  constexpr auto kIndexCountSize{sizeof(uindex)};
  constexpr auto kTextureTupleCountSize{sizeof(uindex)};
  constexpr auto kVertexSize{sizeof(Vertex)};
  constexpr auto kIndexSize{sizeof(Index)};
  constexpr auto kTextureTuplesSize{sizeof(TextureTuple)};

  std::memcpy(&buffer[cursor], reinterpret_cast<const void*>(&model.id),
              kResourceIdSize);
  cursor += kResourceIdSize;

  std::memcpy(&buffer[cursor], reinterpret_cast<const void*>(&model.type_id),
              kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  for (const auto& mesh : model.meshes) {
    std::memcpy(&buffer[cursor],
                reinterpret_cast<const void*>(&mesh.resource_id),
                kResourceIdSize);
    cursor += kResourceIdSize;

    const auto vertex_count{mesh.vertices.size()};
    const auto index_count{mesh.indices.size()};

    std::memcpy(&buffer[cursor], reinterpret_cast<const void*>(&vertex_count),
                kVertexCountSize);

    cursor += kVertexCountSize;
    const auto vertex_total_size{kVertexSize * vertex_count};

    std::memcpy(&buffer[cursor], mesh.vertices.data(), vertex_total_size);

    cursor += vertex_total_size;

    std::memcpy(&buffer[cursor], reinterpret_cast<const void*>(&index_count),
                kIndexCountSize);

    cursor += kIndexCountSize;
    const auto index_total_size{kIndexSize * index_count};

    std::memcpy(&buffer[cursor], mesh.indices.data(), index_total_size);
    cursor += index_total_size;

    const auto texture_tuple_count{mesh.textures.size()};
    std::memcpy(&buffer[cursor],
                reinterpret_cast<const void*>(&texture_tuple_count),
                kTextureTupleCountSize);

    cursor += kTextureTupleCountSize;
    const auto texture_tuple_total_size{kTextureTuplesSize *
                                        texture_tuple_count};

    std::memcpy(&buffer[cursor], mesh.textures.data(),
                texture_tuple_total_size);
    cursor += texture_tuple_total_size;
  }

  PackResourceDescr(model.descr, file);
  PackResourceData(data, file);
  return file;
}

std::unique_ptr<Resource> ModelHandler::Unpack(const ResourceFile& file) const {
  ModelResource&& model{};
  model.descr = UnpackResourceDescr<ModelResourceDescr>(file);

  const auto data{UnpackResourceData(file)};
  auto data_size{sizeof(char) * data.size()};
  const auto* buffer{data.data()};
  uindex cursor{0};
  constexpr auto kResourceIdSize{sizeof(stringid::StringId)};
  constexpr auto kResourceTypeIdSize{sizeof(stringid::StringId)};
  constexpr auto kVertexCountSize{sizeof(uindex)};
  constexpr auto kIndexCountSize{sizeof(uindex)};
  constexpr auto kTextureTupleCountSize{sizeof(uindex)};
  constexpr auto kVertexSize{sizeof(Vertex)};
  constexpr auto kIndexSize{sizeof(Index)};
  constexpr auto kTextureTupleSize{sizeof(TextureTuple)};

  std::memcpy(reinterpret_cast<void*>(&model.id),
              reinterpret_cast<const void*>(&buffer[cursor]), kResourceIdSize);
  cursor += kResourceIdSize;

  std::memcpy(reinterpret_cast<void*>(&model.type_id),
              reinterpret_cast<const void*>(&buffer[cursor]),
              kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  while (cursor < data_size) {
    MeshResource&& mesh{};

    std::memcpy(reinterpret_cast<void*>(&mesh.resource_id),
                reinterpret_cast<const void*>(&buffer[cursor]),
                kResourceIdSize);
    cursor += kResourceIdSize;

    uindex vertex_count{0};
    std::memcpy(reinterpret_cast<void*>(&vertex_count),
                reinterpret_cast<const void*>(&buffer[cursor]),
                kVertexCountSize);
    cursor += kVertexCountSize;

    mesh.vertices.resize(vertex_count);
    const auto vertex_total_size{kVertexSize * vertex_count};
    std::memcpy(reinterpret_cast<void*>(mesh.vertices.data()),
                reinterpret_cast<const void*>(&buffer[cursor]),
                vertex_total_size);
    cursor += vertex_total_size;

    uindex index_count{0};
    std::memcpy(reinterpret_cast<void*>(&index_count),
                reinterpret_cast<const void*>(&buffer[cursor]),
                kIndexCountSize);
    cursor += kIndexCountSize;

    mesh.indices.resize(index_count);
    const auto index_total_size{kIndexSize * index_count};
    std::memcpy(reinterpret_cast<void*>(mesh.indices.data()),
                reinterpret_cast<const void*>(&buffer[cursor]),
                index_total_size);
    cursor += index_total_size;

    uindex texture_tuple_count{0};
    std::memcpy(reinterpret_cast<void*>(&texture_tuple_count),
                reinterpret_cast<const void*>(&buffer[cursor]),
                kTextureTupleCountSize);
    cursor += kTextureTupleCountSize;

    mesh.textures.resize(texture_tuple_count);
    const auto texture_tuple_total_size{kTextureTupleSize *
                                        texture_tuple_count};
    std::memcpy(reinterpret_cast<void*>(mesh.textures.data()),
                reinterpret_cast<const void*>(&buffer[cursor]),
                texture_tuple_total_size);
    cursor += texture_tuple_total_size;

    model.meshes.emplace_back(mesh);
  }

  return std::make_unique<ModelResource>(std::move(model));
}
}  // namespace model
}  // namespace resource
}  // namespace comet
