// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
#include "comet/core/memory/memory_utils.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/resource/texture/texture_resource.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/id/string_id.h"
#include "comet/core/id/string_id_allocator.h"
#include "comet/core/type_trait.h"
#include "comet/data/render/label/texture_label.h"

namespace comet {
namespace resource {
const TextureResource::TypeId TextureResource::kResourceTypeId{
    COMET_STRING_ID(TextureResource::kResourceTypeName.data())};

TextureResource::Id TextureResource::GetId() const noexcept { return Id{id}; }

TextureResourceId GetDefaultTextureFromType(
    render::TextureType texture_type) {
  switch (texture_type) {
    case render::TextureType::Diffuse:
      return kDefaultDiffuseTextureId;
    case render::TextureType::Specular:
      return kDefaultSpecularTextureId;
    case render::TextureType::Normal:
      return kDefaultNormalTextureId;
    default:
      COMET_ASSERT(false, "texture_resource::GetDefaultTextureFromType",
                   "unsupported texture type", "texture_type",
                   render::GetTextureTypeLabel(texture_type),
                   "texture_type_value", ToUnderlying(texture_type));
      return TextureResourceId{kFallbackRawResourceId};
  }
}

usize GetTextureResourceSize(const TextureResource& resource) {
  return sizeof(RawResourceId) + sizeof(ResourceTypeId) +
         resource.data.GetSize();
}
ResourceFile PackTextureResource(const TextureResource& resource,
                                          CompressionMode compression_mode, memory::Allocator* allocator) {
  COMET_ASSERT(allocator != nullptr, "resource::PackTextureResource", "allocator is null");
  COMET_ASSERT(resource.type_id == TextureResource::kResourceTypeId,
               "TextureResourceHandler::Pack",
               "texture resource type id is invalid", "resource_type_id",
               resource.type_id, "expected_type_id",
               TextureResource::kResourceTypeId);

  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = TextureResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{allocator};
  file.data = Array<u8>{allocator};

  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
  const auto data_size{resource.data.GetSize()};

  Array<u8> data{allocator};
  data.Resize(GetTextureResourceSize(resource));

  usize cursor{0};
  auto* buffer{data.GetData()};

  memory::CopyMemory(&buffer[cursor], &resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&buffer[cursor], resource.data.GetData(), data_size);
  cursor += data_size;

  COMET_ASSERT(cursor == data.GetSize(), "TextureResourceHandler::Pack",
               "packed texture size mismatch", "cursor", cursor, "data_size",
               data.GetSize());

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

void UnpackTextureResource(const ResourceFile& file,
                                    memory::Allocator* scratch_allocator,
    memory::Allocator* payload_allocator,
                                    TextureResource* resource) {
  COMET_ASSERT(payload_allocator != nullptr, "resource::UnpackTextureResource", "payload allocator is null");
  COMET_ASSERT(resource != nullptr, "TextureResourceHandler::Unpack",
               "texture resource is null");

  UnpackPodResourceDescr<TextureResourceDescr>(file, resource->descr);

  Array<u8> data{scratch_allocator};
  UnpackResourceData(file, data);

  const auto* buffer{data.GetData()};
  usize cursor{0};

  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};

  COMET_ASSERT(data.GetSize() >= kResourceIdSize + kResourceTypeIdSize,
               "TextureResourceHandler::Unpack", "texture data is too small",
               "data_size", data.GetSize());
  const auto payload_size{data.GetSize() - kResourceIdSize -
                          kResourceTypeIdSize};

  memory::CopyMemory(&resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&resource->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;
  COMET_ASSERT(resource->type_id == TextureResource::kResourceTypeId,
               "TextureResourceHandler::Unpack",
               "texture resource type id is invalid", "resource_type_id",
               resource->type_id, "expected_type_id",
               TextureResource::kResourceTypeId);

  resource->data = Array<u8>{payload_allocator};
  resource->data.Resize(payload_size);
  memory::CopyMemory(resource->data.GetData(), &buffer[cursor], payload_size);
  cursor += payload_size;

  COMET_ASSERT(cursor == data.GetSize(), "TextureResourceHandler::Unpack",
               "unpacked texture size mismatch", "cursor", cursor, "data_size",
               data.GetSize());

}


}  // namespace resource
}  // namespace comet