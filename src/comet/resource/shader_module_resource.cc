// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "shader_module_resource.h"

#include "comet/core/memory/memory_utils.h"

namespace comet {
namespace resource {
const ResourceTypeId ShaderModuleResource::kResourceTypeId{
    COMET_STRING_ID("shader_module")};

ShaderModuleHandler::ShaderModuleHandler(
    memory::Allocator* loading_resources_allocator,
    memory::Allocator* loading_resource_allocator)
    : ResourceHandler{sizeof(ShaderModuleHandler), loading_resources_allocator,
                      loading_resource_allocator} {}

void ShaderModuleHandler::Initialize() {
  ResourceHandler::Initialize();
  resource_data_allocator_.Initialize();
}

void ShaderModuleHandler::Shutdown() {
  resource_data_allocator_.Destroy();
  ResourceHandler::Shutdown();
}

ResourceFile ShaderModuleHandler::Pack(memory::Allocator& allocator,
                                       const Resource& resource,
                                       CompressionMode compression_mode) const {
  const auto& shader_module{static_cast<const ShaderModuleResource&>(resource)};
  ResourceFile file{};
  file.resource_id = shader_module.id;
  file.resource_type_id = ShaderModuleResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{&allocator};
  file.data = Array<u8>{&allocator};

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{shader_module.data.GetSize()};
  Array<u8> data{&allocator};
  data.Resize(kResourceIdSize + kResourceTypeIdSize + data_size);
  usize cursor{0};
  auto* buffer{data.GetData()};

  memory::CopyMemory(&buffer[cursor], &shader_module.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &shader_module.type_id,
                     kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&buffer[cursor], shader_module.data.GetData(), data_size);
  cursor += data_size;

  PackPodResourceDescr(shader_module.descr, file);
  PackResourceData(data, file);
  return file;
}

Resource* ShaderModuleHandler::Unpack(memory::Allocator& allocator,
                                      const ResourceFile& file) {
  auto* shader_module{
      resource_allocator_.AllocateOneAndPopulate<ShaderModuleResource>()};
  UnpackPodResourceDescr<ShaderModuleResourceDescr>(file, shader_module->descr);

  Array<u8> data{&allocator};
  UnpackResourceData(file, data);
  const auto* buffer{data.GetData()};
  usize cursor{0};

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{data.GetSize() - kResourceIdSize - kResourceTypeIdSize};

  memory::CopyMemory(&shader_module->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&shader_module->type_id, &buffer[cursor],
                     kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  shader_module->data = Array<u8>{&resource_data_allocator_};
  shader_module->data.Resize(data_size);
  memory::CopyMemory(shader_module->data.GetData(), &buffer[cursor], data_size);
  cursor += data_size;

  return shader_module;
}
}  // namespace resource
}  // namespace comet
