// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "shader_module_resource_handler.h"

#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"

namespace comet {
namespace resource {
ShaderModuleResourceHandler::ShaderModuleResourceHandler(
    const ResourceHandlerDescr& descr)
    : ResourceHandler<ShaderModuleResource>{descr} {}

ResourceFile ShaderModuleResourceHandler::Pack(
    const ShaderModuleResource& resource, CompressionMode compression_mode) {
  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = ShaderModuleResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{byte_allocator_};
  file.data = Array<u8>{byte_allocator_};

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{resource.data.GetSize()};
  Array<u8> data{byte_allocator_};
  data.Resize(kResourceIdSize + kResourceTypeIdSize + data_size);
  usize cursor{0};
  auto* buffer{data.GetData()};

  memory::CopyMemory(&buffer[cursor], &resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&buffer[cursor], resource.data.GetData(), data_size);
  cursor += data_size;

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

void ShaderModuleResourceHandler::Unpack(const ResourceFile& file,
                                         ResourceLifeSpan life_span,
                                         ShaderModuleResource* resource) {
  UnpackPodResourceDescr<ShaderModuleResourceDescr>(file, resource->descr);

  Array<u8> data{byte_allocator_};
  UnpackResourceData(file, data);
  const auto* buffer{data.GetData()};
  usize cursor{0};

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{data.GetSize() - kResourceIdSize - kResourceTypeIdSize};

  memory::CopyMemory(&resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&resource->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  resource->data = Array<u8>{ResolveAllocator(byte_allocator_, life_span)};
  resource->data.Resize(data_size);
  memory::CopyMemory(resource->data.GetData(), &buffer[cursor], data_size);
  cursor += data_size;
  resource->life_span = life_span;
}
}  // namespace resource
}  // namespace comet
