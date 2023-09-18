// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "shader_module_resource.h"

#include "comet/core/memory/memory.h"

namespace comet {
namespace resource {
const ResourceTypeId ShaderModuleResource::kResourceTypeId{
    COMET_STRING_ID("shader_module")};

ResourceFile ShaderModuleHandler::Pack(const Resource& resource,
                                       CompressionMode compression_mode) const {
  const auto& shader_module{static_cast<const ShaderModuleResource&>(resource)};
  ResourceFile file{};
  file.resource_id = shader_module.id;
  file.resource_type_id = ShaderModuleResource::kResourceTypeId;
  file.compression_mode = compression_mode;

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{shader_module.data.size()};
  std::vector<u8> data(kResourceIdSize + kResourceTypeIdSize + data_size);
  uindex cursor{0};
  auto* buffer{data.data()};

  CopyMemory(&buffer[cursor], &shader_module.id, kResourceIdSize);
  cursor += kResourceIdSize;

  CopyMemory(&buffer[cursor], &shader_module.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  CopyMemory(&buffer[cursor], shader_module.data.data(), data_size);
  cursor += data_size;

  PackPodResourceDescr(shader_module.descr, file);
  PackResourceData(data, file);
  return file;
}

std::unique_ptr<Resource> ShaderModuleHandler::Unpack(
    const ResourceFile& file) const {
  ShaderModuleResource shader_module{};
  shader_module.descr = UnpackPodResourceDescr<ShaderModuleResourceDescr>(file);

  const auto data{UnpackResourceData(file)};
  const auto* buffer{data.data()};
  uindex cursor{0};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{data.size() - kResourceIdSize - kResourceTypeIdSize};

  CopyMemory(&shader_module.id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  CopyMemory(&shader_module.type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  shader_module.data.resize(data_size);
  CopyMemory(shader_module.data.data(), &buffer[cursor], data_size);
  cursor += data_size;

  return std::make_unique<ShaderModuleResource>(std::move(shader_module));
}
}  // namespace resource
}  // namespace comet
