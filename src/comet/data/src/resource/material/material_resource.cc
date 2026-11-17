// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/resource/material/material_resource.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/id/string_id_allocator.h"
#include "comet/core/id/string_id.h"
#include "comet/core/memory/memory_utils.h"

namespace comet {
namespace resource {
const MaterialResource::TypeId MaterialResource::kResourceTypeId{
    COMET_STRING_ID(MaterialResource::kResourceTypeName.data())};

MaterialResourceId GenerateMaterialId(const schar* qualified_name) {
  COMET_ASSERT(qualified_name != nullptr,
               "material_resource::GenerateMaterialId",
               "material qualified name is null");
  return MaterialResourceId{COMET_STRING_ID(qualified_name)};
}

MaterialResourceId GenerateMaterialId(const wchar* qualified_name) {
  COMET_ASSERT(qualified_name != nullptr,
               "material_resource::GenerateMaterialId",
               "material qualified name is null");
  return MaterialResourceId{COMET_STRING_ID(qualified_name)};
}

MaterialResourceId GenerateMaterialId(CTStringView qualified_name) {
  COMET_ASSERT(!qualified_name.IsEmpty(),
               "material_resource::GenerateMaterialId",
               "material qualified name is empty");
  return MaterialResourceId{COMET_STRING_ID(qualified_name)};
}

MaterialResourceId GenerateQualifiedMaterialId(CTStringView file_path,
                                               const schar* material_name,
                                               u32 material_index) {
  COMET_ASSERT(!file_path.IsEmpty(),
               "material_resource::GenerateQualifiedMaterialId",
               "material file path is empty");

  constexpr auto* kDefaultMaterialPrefix{"#material_"};
  constexpr auto kDefaultMaterialPrefixLen{GetLength(kDefaultMaterialPrefix)};
  constexpr auto kMaterialIndexLen{GetCharCount<u32>()};

  schar fallback_name[kDefaultMaterialPrefixLen + kMaterialIndexLen + 1]{'\0'};

  if (material_name == nullptr || IsEmpty(material_name)) {
    Copy(fallback_name, kDefaultMaterialPrefix, kDefaultMaterialPrefixLen);

    schar number[kMaterialIndexLen + 1]{'\0'};
    usize out{0};
    ConvertToStr(material_index, number, kMaterialIndexLen, &out);
    Copy(fallback_name, number, out, kDefaultMaterialPrefixLen);

    material_name = fallback_name;
  }

  const auto material_name_len{GetLength(material_name)};
  const auto path_len{file_path.GetLength()};

  constexpr auto kIndexBufferSize{16};
  schar index_buffer[kIndexBufferSize]{'\0'};
  usize index_len{0};
  ConvertToStr(material_index, index_buffer, kIndexBufferSize, &index_len);

  constexpr usize kResourceTypeNameLen{
      MaterialResource::kResourceTypeName.size()};
  const auto total_len{path_len + 1 + kResourceTypeNameLen + 1 + index_len + 1 +
                       material_name_len};

  constexpr auto kMaxStackBufferSize{512};
  schar* buffer{nullptr};
  memory::UniquePtr<schar[]> heap_buffer{};
  schar stack_buffer[kMaxStackBufferSize]{'\0'};
  constexpr schar kAttributeSeparator{'|'};

  if (total_len + 1 <= kMaxStackBufferSize) {
    buffer = stack_buffer;
  } else {
    heap_buffer.reset(new schar[total_len + 1]);
    buffer = heap_buffer.get();
  }

  usize cursor{0};
  Copy(buffer + cursor, file_path.GetCTStr(), path_len);
  cursor += path_len;
  buffer[cursor++] = kAttributeSeparator;

  Copy(buffer + cursor, MaterialResource::kResourceTypeName.data(),
       kResourceTypeNameLen);
  cursor += kResourceTypeNameLen;
  buffer[cursor++] = kAttributeSeparator;

  Copy(buffer + cursor, index_buffer, index_len);
  cursor += index_len;
  buffer[cursor++] = kAttributeSeparator;

  Copy(buffer + cursor, material_name, material_name_len);
  cursor += material_name_len;

  buffer[cursor] = '\0';
  return MaterialResourceId{COMET_STRING_ID(buffer)};
}

MaterialResource::Id MaterialResource::GetId() const noexcept { return Id{id}; }

ResourceFile PackMaterialResource(const MaterialResource& resource,
                                  CompressionMode compression_mode,
                                  memory::Allocator* allocator) {
  COMET_ASSERT(allocator != nullptr, "resource::PackMaterialResource",
               "allocator is null");
  COMET_ASSERT(resource.type_id == MaterialResource::kResourceTypeId,
               "resource::PackMaterialResource",
               "material resource type id is invalid");

  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = MaterialResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{allocator};
  file.data = Array<u8>{allocator};

  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};

  Array<u8> data{allocator};
  data.Resize(GetMaterialResourceSize(resource));
  usize cursor{0};
  auto* buffer{data.GetData()};

  memory::CopyMemory(&buffer[cursor], &resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  COMET_ASSERT(cursor == data.GetSize(), "resource::PackMaterialResource",
               "packed material size mismatch", "cursor", cursor,
               "data_size", data.GetSize());

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

usize GetMaterialResourceSize(const MaterialResource&) {
  return sizeof(RawResourceId) + sizeof(ResourceTypeId);
}
void UnpackMaterialResource(const ResourceFile& file,
                                     memory::Allocator* scratch_allocator,
    memory::Allocator* payload_allocator,
                                     MaterialResource* resource) {
  COMET_ASSERT(resource != nullptr, "MaterialResourceHandler::Unpack",
               "material resource is null");

  UnpackPodResourceDescr<MaterialResourceDescr>(file, resource->descr);

  (void)payload_allocator;
  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
  constexpr auto kDataSize{kResourceIdSize + kResourceTypeIdSize};
  COMET_ASSERT(scratch_allocator != nullptr, "resource::UnpackMaterialResource", "scratch allocator is null");
  Array<u8> data{scratch_allocator};
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

}


}  // namespace resource
}  // namespace comet