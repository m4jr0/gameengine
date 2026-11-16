// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/shader/shader_module_resource_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/container/array.h"

namespace comet {
namespace resource {
ShaderModuleResourceHandler::ShaderModuleResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

ResourceFile ShaderModuleResourceHandler::Pack(const ShaderModuleResource& resource,
    CompressionMode compression_mode) {
  return PackShaderModuleResource(resource, compression_mode, byte_allocator_);
}

void ShaderModuleResourceHandler::Unpack(const ResourceFile& file,
                                         ResourceLifeSpan life_span,
                                         ShaderModuleResource* resource) {
  COMET_ASSERT(resource != nullptr, "ShaderModuleResourceHandler::Unpack",
               "shader module resource is null");

  UnpackPodResourceDescr<ShaderModuleResourceDescr>(file, resource->descr);

  Array<u8> data{byte_allocator_};
  UnpackResourceData(file, data);
  const auto* buffer{data.GetData()};
  usize cursor{0};

  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};

  COMET_ASSERT(data.GetSize() >= kResourceIdSize + kResourceTypeIdSize,
               "ShaderModuleResourceHandler::Unpack",
               "shader module data is too small", "data_size", data.GetSize());
  const auto data_size{data.GetSize() - kResourceIdSize - kResourceTypeIdSize};

  memory::CopyMemory(&resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&resource->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;
  COMET_ASSERT(resource->type_id == ShaderModuleResource::kResourceTypeId,
               "ShaderModuleResourceHandler::Unpack",
               "shader module resource type id is invalid", "resource_type_id",
               resource->type_id, "expected_type_id",
               ShaderModuleResource::kResourceTypeId);

  resource->data = Array<u8>{ResolveAllocator(byte_allocator_, life_span)};
  resource->data.Resize(data_size);
  memory::CopyMemory(resource->data.GetData(), &buffer[cursor], data_size);
  cursor += data_size;

  COMET_ASSERT(cursor == data.GetSize(), "ShaderModuleResourceHandler::Unpack",
               "unpacked shader module size mismatch", "cursor", cursor,
               "data_size", data.GetSize());

  resource->life_span = life_span;
}
}  // namespace resource
}  // namespace comet
