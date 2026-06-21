// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/material/material_resource_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/runtime/memory/allocator/stack_allocator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/container/array.h"
#include "comet/data/render/texture.h"
#include "comet/data/resource/texture/texture_resource.h"

namespace comet {
namespace resource {
MaterialResourceHandler::MaterialResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

ResourceFile MaterialResourceHandler::Pack(const MaterialResource& resource,
                                           CompressionMode compression_mode) {
  COMET_ASSERT(resource.type_id == MaterialResource::kResourceTypeId,
               "MaterialResourceHandler::Pack",
               "material resource type id is invalid", "resource_type_id",
               resource.type_id, "expected_type_id",
               MaterialResource::kResourceTypeId);

  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = MaterialResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{byte_allocator_};
  file.data = Array<u8>{byte_allocator_};

  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};

  Array<u8> data{byte_allocator_};
  data.Resize(GetMaterialResourceSize(resource));
  usize cursor{0};
  auto* buffer{data.GetData()};

  memory::CopyMemory(&buffer[cursor], &resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  COMET_ASSERT(cursor == data.GetSize(), "MaterialResourceHandler::Pack",
               "packed material size mismatch", "cursor", cursor, "data_size",
               data.GetSize());

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

void MaterialResourceHandler::Unpack(const ResourceFile& file,
                                     ResourceLifeSpan life_span,
                                     MaterialResource* resource) {
  COMET_ASSERT(resource != nullptr, "MaterialResourceHandler::Unpack",
               "material resource is null");

  UnpackPodResourceDescr<MaterialResourceDescr>(file, resource->descr);

  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
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
  COMET_ASSERT(resource->type_id == MaterialResource::kResourceTypeId,
               "MaterialResourceHandler::Unpack",
               "material resource type id is invalid", "resource_type_id",
               resource->type_id, "expected_type_id",
               MaterialResource::kResourceTypeId);

  COMET_ASSERT(cursor == data.GetSize(), "MaterialResourceHandler::Unpack",
               "unpacked material size mismatch", "cursor", cursor, "data_size",
               data.GetSize());

  resource->life_span = life_span;
}

MaterialResource* MaterialResourceHandler::GetDefaultMaterialResource() {
  if (default_material_ == nullptr) {
    default_material_ = std::make_unique<MaterialResource>();
    default_material_->id = kFallbackRawResourceId;
    default_material_->type_id = MaterialResource::kResourceTypeId;

    auto& descr{default_material_->descr};
    descr.diffuse_map.texture_resource_id = kDefaultDiffuseTextureId;
    descr.diffuse_map.type = render::TextureType::Diffuse;
    descr.diffuse_map.u_repeat_mode = render::TextureRepeatMode::Repeat;
    descr.diffuse_map.v_repeat_mode = render::TextureRepeatMode::Repeat;
    descr.diffuse_map.w_repeat_mode = render::TextureRepeatMode::Repeat;
    descr.diffuse_map.min_filter_mode = render::TextureFilterMode::Linear;
    descr.diffuse_map.mag_filter_mode = render::TextureFilterMode::Linear;

    descr.specular_map.texture_resource_id = kDefaultSpecularTextureId;
    descr.specular_map.type = render::TextureType::Specular;
    descr.specular_map.u_repeat_mode = render::TextureRepeatMode::Repeat;
    descr.specular_map.v_repeat_mode = render::TextureRepeatMode::Repeat;
    descr.specular_map.w_repeat_mode = render::TextureRepeatMode::Repeat;
    descr.specular_map.min_filter_mode = render::TextureFilterMode::Linear;
    descr.specular_map.mag_filter_mode = render::TextureFilterMode::Linear;

    descr.normal_map.texture_resource_id = kDefaultNormalTextureId;
    descr.normal_map.type = render::TextureType::Normal;
    descr.normal_map.u_repeat_mode = render::TextureRepeatMode::Repeat;
    descr.normal_map.v_repeat_mode = render::TextureRepeatMode::Repeat;
    descr.normal_map.w_repeat_mode = render::TextureRepeatMode::Repeat;
    descr.normal_map.min_filter_mode = render::TextureFilterMode::Linear;
    descr.normal_map.mag_filter_mode = render::TextureFilterMode::Linear;
  }

  return default_material_.get();
}

void MaterialResourceHandler::InitializeDefaults() {
  COMET_ASSERT(defaults_.IsEmpty(),
               "MaterialResourceHandler::InitializeDefaults",
               "default resource list is not empty");

  defaults_.Reserve(1);
  RegisterDefaultResource(GetDefaultMaterialResource());
}

void MaterialResourceHandler::DestroyDefaults() { default_material_.reset(); }
}  // namespace resource
}  // namespace comet
