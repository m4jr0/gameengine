// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "material_resource.h"

namespace comet {
namespace resource {
const ResourceTypeId MaterialResource::kResourceTypeId{
    GenerateResourceTypeId("material")};

ResourceFile MaterialHandler::Pack(const Resource& resource,
                                   CompressionMode compression_mode) const {
  const auto& material{static_cast<const MaterialResource&>(resource)};
  ResourceFile file{};
  file.resource_id = material.id;
  file.resource_type_id = MaterialResource::kResourceTypeId;
  file.compression_mode = compression_mode;

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kTextureTupleTypeSize{sizeof(resource::TextureTuple)};
  const auto texture_tuples_size{kTextureTupleTypeSize *
                                 material.texture_tuples.size()};

  std::vector<u8> data(kResourceIdSize + kResourceTypeIdSize +
                       texture_tuples_size);
  uindex cursor{0};
  auto* buffer{data.data()};

  std::memcpy(&buffer[cursor], reinterpret_cast<const void*>(&material.id),
              kResourceIdSize);
  cursor += kResourceIdSize;

  std::memcpy(&buffer[cursor], reinterpret_cast<const void*>(&material.type_id),
              kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  std::memcpy(&buffer[cursor],
              reinterpret_cast<const void*>(material.texture_tuples.data()),
              texture_tuples_size);
  cursor += texture_tuples_size;

  PackResourceDescr(material.descr, file);
  PackResourceData(data, file);
  return file;
}

std::unique_ptr<Resource> MaterialHandler::Unpack(
    const ResourceFile& file) const {
  MaterialResource&& material{};
  material.descr = UnpackResourceDescr<MaterialResourceDescr>(file);
  const auto data{UnpackResourceData(file)};

  const auto* buffer{data.data()};
  uindex cursor{0};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kTextureTupleTypeSize{sizeof(resource::TextureTuple)};
  const auto texture_tuples_size{sizeof(u8) * data.size() - kResourceIdSize -
                                 kResourceTypeIdSize};
  const uindex texture_tuples_count{texture_tuples_size /
                                    kTextureTupleTypeSize};

  std::memcpy(reinterpret_cast<void*>(&material.id),
              reinterpret_cast<const void*>(&buffer[cursor]), kResourceIdSize);
  cursor += kResourceIdSize;

  std::memcpy(reinterpret_cast<void*>(&material.type_id),
              reinterpret_cast<const void*>(&buffer[cursor]),
              kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  material.texture_tuples.resize(texture_tuples_count);
  std::memcpy(reinterpret_cast<void*>(material.texture_tuples.data()),
              reinterpret_cast<const void*>(&buffer[cursor]),
              texture_tuples_size);
  cursor += texture_tuples_size;

  return std::make_unique<MaterialResource>(std::move(material));
}

resource::MaterialId GenerateMaterialId(const std::string& material_name) {
  return GenerateMaterialId(material_name.c_str());
}

resource::MaterialId GenerateMaterialId(const char* material_name) {
  return COMET_STRING_ID(material_name);
}
}  // namespace resource
}  // namespace comet
