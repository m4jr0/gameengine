// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "material_resource_handler.h"

#include "comet/core/memory/allocator/stack_allocator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace resource {
MaterialResourceHandler::MaterialResourceHandler(
    const ResourceHandlerDescr& descr)
    : ResourceHandler<MaterialResource>{descr} {}

void MaterialResourceHandler::InitializeDefaults() {
  defaults_.Reserve(1);
  defaults_.Set(GetDefaultMaterialResource());
}

void MaterialResourceHandler::DestroyDefaults() { defaults_.Destroy(); }

ResourceFile MaterialResourceHandler::Pack(const MaterialResource& resource,
                                           CompressionMode compression_mode) {
  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = MaterialResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{byte_allocator_};
  file.data = Array<u8>{byte_allocator_};

  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};

  Array<u8> data{byte_allocator_};
  data.Resize(kResourceIdSize + kResourceTypeIdSize);
  usize cursor{0};
  auto* buffer{data.GetData()};

  memory::CopyMemory(&buffer[cursor], &resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

void MaterialResourceHandler::Unpack(const ResourceFile& file,
                                     ResourceLifeSpan life_span,
                                     MaterialResource* resource) {
  UnpackPodResourceDescr<MaterialResourceDescr>(file, resource->descr);

  constexpr auto kResourceIdSize{sizeof(ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
  constexpr auto kDataSize{kResourceIdSize + kResourceTypeIdSize};
  memory::StaticStackAllocator<kDataSize> tmp_allocator{};

  Array<u8> data{&tmp_allocator};
  data.Resize(kDataSize);
  UnpackResourceData(file, data, kDataSize);

  const auto* buffer{data.GetData()};
  usize cursor{0};

  memory::CopyMemory(&resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&resource->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;
  resource->life_span = life_span;
}

MaterialResource* MaterialResourceHandler::GetDefaultMaterialResource() {
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
