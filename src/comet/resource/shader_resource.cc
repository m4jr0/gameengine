// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "shader_resource.h"

namespace comet {
namespace resource {
const ResourceTypeId ShaderResource::kResourceTypeId{
    GenerateResourceTypeId("shader")};

ResourceFile ShaderHandler::Pack(const Resource& resource,
                                 CompressionMode compression_mode) const {
  const auto& shader{static_cast<const ShaderResource&>(resource)};
  ResourceFile file{};
  file.resource_id = shader.id;
  file.resource_type_id = ShaderResource::kResourceTypeId;
  file.compression_mode = compression_mode;

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{shader.data.size()};
  std::vector<u8> data(kResourceIdSize + kResourceTypeIdSize + data_size);
  uindex cursor{0};
  auto* buffer{data.data()};

  std::memcpy(&buffer[cursor], reinterpret_cast<const void*>(&shader.id),
              kResourceIdSize);
  cursor += kResourceIdSize;

  std::memcpy(&buffer[cursor], reinterpret_cast<const void*>(&shader.type_id),
              kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  std::memcpy(&buffer[cursor],
              reinterpret_cast<const void*>(shader.data.data()), data_size);
  cursor += data_size;

  PackResourceDescr(shader.descr, file);
  PackResourceData(data, file);
  return file;
}

std::unique_ptr<Resource> ShaderHandler::Unpack(
    const ResourceFile& file) const {
  ShaderResource&& shader{};
  shader.descr = UnpackResourceDescr<ShaderResourceDescr>(file);

  const auto data{UnpackResourceData(file)};
  const auto* buffer{data.data()};
  uindex cursor{0};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{data.size() - kResourceIdSize - kResourceTypeIdSize};

  std::memcpy(reinterpret_cast<void*>(&shader.id),
              reinterpret_cast<const void*>(&buffer[cursor]), kResourceIdSize);
  cursor += kResourceIdSize;

  std::memcpy(reinterpret_cast<void*>(&shader.type_id),
              reinterpret_cast<const void*>(&buffer[cursor]),
              kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  shader.data.resize(data_size);
  std::memcpy(reinterpret_cast<void*>(shader.data.data()),
              reinterpret_cast<const void*>(&buffer[cursor]), data_size);
  cursor += data_size;

  return std::make_unique<ShaderResource>(std::move(shader));
}
}  // namespace resource
}  // namespace comet
