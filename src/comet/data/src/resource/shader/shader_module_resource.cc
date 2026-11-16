// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
#include "comet/core/memory/memory_utils.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/resource/shader/shader_module_resource.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/id/string_id.h"
#include "comet/core/id/string_id_allocator.h"

namespace comet {
namespace resource {
const ShaderModuleResource::TypeId ShaderModuleResource::kResourceTypeId{
    COMET_STRING_ID(ShaderModuleResource::kResourceTypeName.data())};

ShaderModuleResource::Id ShaderModuleResource::GetId() const noexcept {
  return Id{id};
}

usize GetShaderModuleResourceSize(const ShaderModuleResource& resource) {
  return sizeof(RawResourceId) + sizeof(ResourceTypeId) +
         resource.data.GetSize();
}
ResourceFile PackShaderModuleResource(
    const ShaderModuleResource& resource, CompressionMode compression_mode, memory::Allocator* allocator) {
  COMET_ASSERT(allocator != nullptr, "resource::PackShaderModuleResource", "allocator is null");
  COMET_ASSERT(resource.type_id == ShaderModuleResource::kResourceTypeId,
               "ShaderModuleResourceHandler::Pack",
               "shader module resource type id is invalid", "resource_type_id",
               resource.type_id, "expected_type_id",
               ShaderModuleResource::kResourceTypeId);

  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = ShaderModuleResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{allocator};
  file.data = Array<u8>{allocator};

  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
  const auto data_size{resource.data.GetSize()};
  Array<u8> data{allocator};
  data.Resize(GetShaderModuleResourceSize(resource));
  usize cursor{0};
  auto* buffer{data.GetData()};

  memory::CopyMemory(&buffer[cursor], &resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&buffer[cursor], resource.data.GetData(), data_size);
  cursor += data_size;

  COMET_ASSERT(cursor == data.GetSize(), "ShaderModuleResourceHandler::Pack",
               "packed shader module size mismatch", "cursor", cursor,
               "data_size", data.GetSize());

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

}  // namespace resource
}  // namespace comet
