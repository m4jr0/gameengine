// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "texture_resource.h"

namespace comet {
namespace resource {
const ResourceTypeId TextureResource::kResourceTypeId{
    GenerateResourceTypeId("texture")};

ResourceFile TextureHandler::Pack(const Resource& resource,
                                  CompressionMode compression_mode) const {
  const auto& texture{static_cast<const TextureResource&>(resource)};
  ResourceFile file{};
  file.resource_id = texture.id;
  file.resource_type_id = TextureResource::kResourceTypeId;
  file.compression_mode = compression_mode;

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{sizeof(u8) * texture.data.size()};

  std::vector<u8> data(kResourceIdSize + kResourceTypeIdSize + data_size);
  uindex cursor{0};
  auto* buffer{data.data()};

  std::memcpy(&buffer[cursor], reinterpret_cast<const void*>(&texture.id),
              kResourceIdSize);
  cursor += kResourceIdSize;

  std::memcpy(&buffer[cursor], reinterpret_cast<const void*>(&texture.type_id),
              kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  std::memcpy(&buffer[cursor],
              reinterpret_cast<const void*>(texture.data.data()), data_size);
  cursor += data_size;

  PackResourceDescr(texture.descr, file);
  PackResourceData(data, file);
  return file;
}

std::unique_ptr<Resource> TextureHandler::Unpack(
    const ResourceFile& file) const {
  TextureResource&& texture{};
  texture.descr = UnpackResourceDescr<TextureResourceDescr>(file);
  const auto data{UnpackResourceData(file)};

  const auto* buffer{data.data()};
  uindex cursor{0};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{sizeof(u8) * data.size() - kResourceIdSize -
                       kResourceTypeIdSize};

  std::memcpy(reinterpret_cast<void*>(&texture.id),
              reinterpret_cast<const void*>(&buffer[cursor]), kResourceIdSize);
  cursor += kResourceIdSize;

  std::memcpy(reinterpret_cast<void*>(&texture.type_id),
              reinterpret_cast<const void*>(&buffer[cursor]),
              kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  texture.data.resize(data_size);
  std::memcpy(reinterpret_cast<void*>(texture.data.data()),
              reinterpret_cast<const void*>(&buffer[cursor]), data_size);
  cursor += data_size;

  return std::make_unique<TextureResource>(std::move(texture));
}
}  // namespace resource
}  // namespace comet
