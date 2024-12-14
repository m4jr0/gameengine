// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "material_resource.h"

#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace resource {
const ResourceTypeId MaterialResource::kResourceTypeId{
    COMET_STRING_ID("material")};

ResourceId GenerateMaterialId(const schar* material_name) {
  return COMET_STRING_ID(material_name);
}

ResourceFile MaterialHandler::Pack(memory::Allocator& allocator,
                                   const Resource& resource,
                                   CompressionMode compression_mode) const {
  const auto& material{static_cast<const MaterialResource&>(resource)};
  ResourceFile file{};
  file.resource_id = material.id;
  file.resource_type_id = MaterialResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{&allocator};
  file.data = Array<u8>{&allocator};

  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};

  Array<u8> data{&allocator};
  data.Resize(kResourceIdSize + kResourceTypeIdSize);
  usize cursor{0};
  auto* buffer{data.GetData()};

  memory::CopyMemory(&buffer[cursor], &material.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &material.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  PackPodResourceDescr(material.descr, file);
  PackResourceData(data, file);
  return file;
}

Resource* MaterialHandler::Unpack(memory::Allocator& allocator,
                                  const ResourceFile& file) {
  auto* material{
      resource_allocator_.AllocateOneAndPopulate<MaterialResource>()};
  UnpackPodResourceDescr<MaterialResourceDescr>(file, material->descr);

  Array<u8> data{&allocator};
  UnpackResourceData(file, data);

  const auto* buffer{data.GetData()};
  usize cursor{0};
  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};

  memory::CopyMemory(&material->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&material->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  return material;
}

MaterialHandler::MaterialHandler(memory::Allocator* loading_resources_allocator,
                                 memory::Allocator* loading_resource_allocator)
    : ResourceHandler{sizeof(MaterialResource), loading_resources_allocator,
                      loading_resource_allocator} {}

Resource* MaterialHandler::GetDefaultResource() {
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
