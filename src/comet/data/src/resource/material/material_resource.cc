// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/resource/material/material_resource.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/runtime/frame/frame_string.h"
#include "comet/core/id/string_id_allocator.h"
#include "comet/core/id/string_id.h"

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
  schar stack_buffer[kMaxStackBufferSize]{'\0'};
  constexpr schar kAttributeSeparator{'|'};

  if (total_len + 1 <= kMaxStackBufferSize) {
    buffer = stack_buffer;
  } else {
    buffer = GenerateFrameString<schar>(total_len + 1);
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

usize GetMaterialResourceSize(const MaterialResource&) {
  return sizeof(RawResourceId) + sizeof(ResourceTypeId);
}
}  // namespace resource
}  // namespace comet