// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "material_resource.h"

#include "comet/core/memory/memory.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace resource {
const ResourceTypeId MaterialResource::kResourceTypeId{
    COMET_STRING_ID("material")};

ResourceId GenerateMaterialId(const schar* material_name) {
  return COMET_STRING_ID(material_name);
}

ResourceFile MaterialHandler::Pack(const Resource& resource,
                                   CompressionMode compression_mode) const {
  const auto& material{static_cast<const MaterialResource&>(resource)};
  ResourceFile file{};
  file.resource_id = material.id;
  file.resource_type_id = MaterialResource::kResourceTypeId;
  file.compression_mode = compression_mode;

  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};

  std::vector<u8> data(kResourceIdSize + kResourceTypeIdSize);
  uindex cursor{0};
  auto* buffer{data.data()};

  CopyMemory(&buffer[cursor], &material.id, kResourceIdSize);
  cursor += kResourceIdSize;

  CopyMemory(&buffer[cursor], &material.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  PackPodResourceDescr(material.descr, file);
  PackResourceData(data, file);
  return file;
}

std::unique_ptr<Resource> MaterialHandler::Unpack(
    const ResourceFile& file) const {
  MaterialResource material{};
  material.descr = UnpackPodResourceDescr<MaterialResourceDescr>(file);
  const auto data{UnpackResourceData(file)};

  const auto* buffer{data.data()};
  uindex cursor{0};
  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};

  CopyMemory(&material.id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  CopyMemory(&material.type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  return std::make_unique<MaterialResource>(material);
}

const Resource* MaterialHandler::GetDefaultResource() {
  if (default_material_ == nullptr) {
    default_material_ = std::make_unique<MaterialResource>();
    default_material_->id = kDefaultResourceId;
    default_material_->type_id = MaterialResource::kResourceTypeId;

    auto& descr{default_material_->descr};
    descr.diffuse_map.texture_id = kDefaultDiffuseTextureResourceId;
    descr.diffuse_map.type = rendering::TextureType::Diffuse;
  }

  return default_material_.get();
}
}  // namespace resource
}  // namespace comet
