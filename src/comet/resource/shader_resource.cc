// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "shader_resource.h"

#include <utility>

#include "comet/core/c_string.h"
#include "comet/core/memory/memory_utils.h"

namespace comet {
namespace resource {
const ResourceTypeId ShaderResource::kResourceTypeId{COMET_STRING_ID("shader")};

ShaderHandler::ShaderHandler(memory::Allocator* loading_resources_allocator,
                             memory::Allocator* loading_resource_allocator)
    : ResourceHandler{sizeof(ShaderHandler), loading_resources_allocator,
                      loading_resource_allocator} {}

ResourceFile ShaderHandler::Pack(memory::Allocator& allocator,
                                 const Resource& resource,
                                 CompressionMode compression_mode) const {
  const auto& shader{static_cast<const ShaderResource&>(resource)};
  ResourceFile file{};
  file.resource_id = shader.id;
  file.resource_type_id = ShaderResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{&allocator};
  file.data = Array<u8>{&allocator};

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  Array<u8> data{&allocator};
  data.Resize(kResourceIdSize + kResourceTypeIdSize);
  usize cursor{0};
  auto* buffer{data.GetData()};

  memory::CopyMemory(&buffer[cursor], &shader.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &shader.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  const auto dumped_descr{DumpDescr(allocator, shader.descr)};
  file.descr_size = dumped_descr.GetSize();
  PackBytes(dumped_descr, file.compression_mode, &file.descr,
            &file.packed_descr_size);
  PackResourceData(data, file);
  return file;
}

Resource* ShaderHandler::Unpack(memory::Allocator& allocator,
                                const ResourceFile& file) {
  auto* shader{resource_allocator_.AllocateOneAndPopulate<ShaderResource>()};

  Array<u8> dumped_descr{&allocator};
  UnpackBytes(file.compression_mode, file.descr, file.descr_size, dumped_descr);

  ShaderResourceDescr& descr{shader->descr};
  descr.shader_module_paths = Array<TString>{&allocator};
  descr.vertex_attributes =
      Array<rendering::ShaderVertexAttributeDescr>{&allocator};
  descr.uniforms = Array<rendering::ShaderUniformDescr>{&allocator};
  descr.constants = Array<rendering::ShaderConstantDescr>{&allocator};
  descr.storages = Array<rendering::ShaderStorageDescr>{&allocator};
  ParseDescr(dumped_descr, &allocator, descr);

  Array<u8> data{&allocator};
  UnpackResourceData(file, data);
  const auto* buffer{data.GetData()};
  usize cursor{0};

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};

  memory::CopyMemory(&shader->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&shader->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  return shader;
}

Array<u8> ShaderHandler::DumpDescr(memory::Allocator& allocator,
                                   const ShaderResourceDescr& descr) {
  constexpr auto kBoolSize{sizeof(bool)};
  constexpr auto kCullModeSize{sizeof(rendering::CullMode)};

  const auto data_size{GetSizeFromDescr(descr)};
  Array<u8> dumped_descr{&allocator};
  dumped_descr.Resize(data_size);
  usize cursor{0};
  auto* buffer{dumped_descr.GetData()};

  memory::CopyMemory(&buffer[cursor], &descr.is_wireframe, kBoolSize);
  cursor += kBoolSize;

  memory::CopyMemory(&buffer[cursor], &descr.cull_mode, kCullModeSize);
  cursor += kCullModeSize;

  DumpShaderModules(descr, buffer, cursor);
  DumpVertexAttributes(descr, buffer, cursor);
  DumpUniforms(descr, buffer, cursor);
  DumpConstants(descr, buffer, cursor);
  DumpStorages(descr, buffer, cursor);

  return dumped_descr;
}

usize ShaderHandler::GetSizeFromDescr(const ShaderResourceDescr& descr) {
  constexpr auto kBoolSize{sizeof(bool)};
  constexpr auto kCullModeSize{sizeof(rendering::CullMode)};
  constexpr auto kUsizeSize(sizeof(usize));
  constexpr auto kShaderVertexAttributeTypeSize(
      sizeof(rendering::ShaderVertexAttributeType));
  constexpr auto kShaderUniformTypeSize(sizeof(rendering::ShaderVariableType));
  constexpr auto kShaderUniformScopeSize(sizeof(rendering::ShaderUniformScope));
  constexpr auto kShaderConstantTypeSize(sizeof(rendering::ShaderVariableType));
  constexpr auto kShaderStoragePropertyTypeSize(
      sizeof(rendering::ShaderVariableType));
  constexpr auto kShaderStageFlagsSize(sizeof(rendering::ShaderStageFlags));

  // is_wireframe size, cull_mode size, and shader_module_paths size.
  auto total_size{kBoolSize + kCullModeSize + kUsizeSize};

  for (const auto& module_path : descr.shader_module_paths) {
    total_size += kUsizeSize +
                  (module_path.GetLengthWithNullTerminator()) * sizeof(tchar);
  }

  total_size += kUsizeSize;

  for (const auto& vertex_attribute : descr.vertex_attributes) {
    total_size +=
        kShaderVertexAttributeTypeSize + kUsizeSize + vertex_attribute.name_len;
  }

  total_size += kUsizeSize;

  for (const auto& uniform : descr.uniforms) {
    total_size += kShaderUniformTypeSize + kShaderUniformScopeSize +
                  kShaderStageFlagsSize + kUsizeSize + uniform.name_len;
  }

  total_size += kUsizeSize;

  for (const auto& constant : descr.constants) {
    total_size += kShaderConstantTypeSize + kShaderStageFlagsSize + kUsizeSize +
                  constant.name_len;
  }

  total_size += kUsizeSize;

  for (const auto& buffer : descr.storages) {
    total_size += kUsizeSize + buffer.name_len;
    total_size += kShaderStageFlagsSize;
    total_size += kUsizeSize;

    for (const auto& property : buffer.properties) {
      total_size +=
          kShaderStoragePropertyTypeSize + kUsizeSize + property.name_len;
    }
  }

  return total_size;
}

void ShaderHandler::DumpShaderModules(const ShaderResourceDescr& descr,
                                      u8* buffer, usize& cursor) {
  constexpr auto kUsizeSize(sizeof(usize));

  const auto module_path_count{descr.shader_module_paths.GetSize()};
  memory::CopyMemory(&buffer[cursor], &module_path_count, kUsizeSize);
  cursor += kUsizeSize;

  for (const auto& module_path : descr.shader_module_paths) {
    const auto module_path_size{(module_path.GetLengthWithNullTerminator()) *
                                sizeof(tchar)};
    memory::CopyMemory(&buffer[cursor], &module_path_size, kUsizeSize);
    cursor += kUsizeSize;

    memory::CopyMemory(&buffer[cursor], module_path.GetCTStr(),
                       module_path_size);
    cursor += module_path_size;
  }
}

void ShaderHandler::DumpVertexAttributes(const ShaderResourceDescr& descr,
                                         u8* buffer, usize& cursor) {
  constexpr auto kUsizeSize(sizeof(usize));
  constexpr auto kShaderVertexAttributeTypeSize(
      sizeof(rendering::ShaderVertexAttributeType));

  const auto vertex_attribute_count{descr.vertex_attributes.GetSize()};
  memory::CopyMemory(&buffer[cursor], &vertex_attribute_count, kUsizeSize);
  cursor += kUsizeSize;

  for (const auto& vertex_attribute : descr.vertex_attributes) {
    memory::CopyMemory(&buffer[cursor], &vertex_attribute.type,
                       kShaderVertexAttributeTypeSize);
    cursor += kShaderVertexAttributeTypeSize;

    const auto vertex_attribute_name_size{vertex_attribute.name_len};
    memory::CopyMemory(&buffer[cursor], &vertex_attribute_name_size,
                       kUsizeSize);
    cursor += kUsizeSize;

    memory::CopyMemory(&buffer[cursor], vertex_attribute.name,
                       vertex_attribute_name_size);
    cursor += vertex_attribute_name_size;
  }
}

void ShaderHandler::DumpUniforms(const ShaderResourceDescr& descr, u8* buffer,
                                 usize& cursor) {
  constexpr auto kUsizeSize(sizeof(usize));
  constexpr auto kShaderUniformTypeSize(sizeof(rendering::ShaderVariableType));
  constexpr auto kShaderUniformScopeSize(sizeof(rendering::ShaderUniformScope));
  constexpr auto kShaderStageFlagsSize(sizeof(rendering::ShaderStageFlags));

  const auto uniform_count{descr.uniforms.GetSize()};
  memory::CopyMemory(&buffer[cursor], &uniform_count, kUsizeSize);
  cursor += kUsizeSize;

  for (const auto& uniform : descr.uniforms) {
    memory::CopyMemory(&buffer[cursor], &uniform.type, kShaderUniformTypeSize);
    cursor += kShaderUniformTypeSize;

    memory::CopyMemory(&buffer[cursor], &uniform.scope,
                       kShaderUniformScopeSize);
    cursor += kShaderUniformScopeSize;

    memory::CopyMemory(&buffer[cursor], &uniform.stages, kShaderStageFlagsSize);
    cursor += kShaderStageFlagsSize;

    const auto uniform_name_len{uniform.name_len};
    memory::CopyMemory(&buffer[cursor], &uniform_name_len, kUsizeSize);
    cursor += kUsizeSize;

    memory::CopyMemory(&buffer[cursor], uniform.name, uniform_name_len);
    cursor += uniform_name_len;
  }
}

void ShaderHandler::DumpConstants(const ShaderResourceDescr& descr, u8* buffer,
                                  usize& cursor) {
  constexpr auto kUsizeSize(sizeof(usize));
  constexpr auto kShaderConstantTypeSize(sizeof(rendering::ShaderVariableType));
  constexpr auto kShaderStageFlagsSize(sizeof(rendering::ShaderStageFlags));

  const auto constant_count{descr.constants.GetSize()};
  memory::CopyMemory(&buffer[cursor], &constant_count, kUsizeSize);
  cursor += kUsizeSize;

  for (const auto& constant : descr.constants) {
    memory::CopyMemory(&buffer[cursor], &constant.type,
                       kShaderConstantTypeSize);
    cursor += kShaderConstantTypeSize;

    memory::CopyMemory(&buffer[cursor], &constant.stages,
                       kShaderStageFlagsSize);
    cursor += kShaderStageFlagsSize;

    const auto constant_name_len{constant.name_len};
    memory::CopyMemory(&buffer[cursor], &constant_name_len, kUsizeSize);
    cursor += kUsizeSize;

    memory::CopyMemory(&buffer[cursor], constant.name, constant_name_len);
    cursor += constant_name_len;
  }
}

void ShaderHandler::DumpStorages(const ShaderResourceDescr& descr, u8* buffer,
                                 usize& cursor) {
  constexpr auto kUsizeSize(sizeof(usize));
  constexpr auto kShaderStageFlagsSize(sizeof(rendering::ShaderStageFlags));
  constexpr auto kShaderStoragePropertyTypeSize(
      sizeof(rendering::ShaderVariableType));

  const auto storage_count{descr.storages.GetSize()};
  memory::CopyMemory(&buffer[cursor], &storage_count, kUsizeSize);
  cursor += kUsizeSize;

  for (const auto& storage : descr.storages) {
    const auto storage_name_len{storage.name_len};
    memory::CopyMemory(&buffer[cursor], &storage_name_len, kUsizeSize);
    cursor += kUsizeSize;

    memory::CopyMemory(&buffer[cursor], storage.name, storage_name_len);
    cursor += storage_name_len;

    memory::CopyMemory(&buffer[cursor], &storage.stages, kShaderStageFlagsSize);
    cursor += kShaderStageFlagsSize;

    const auto property_count{storage.properties.GetSize()};
    memory::CopyMemory(&buffer[cursor], &property_count, kUsizeSize);
    cursor += kUsizeSize;

    for (const auto& property : storage.properties) {
      memory::CopyMemory(&buffer[cursor], &property.type,
                         kShaderStoragePropertyTypeSize);
      cursor += kShaderStoragePropertyTypeSize;

      const auto property_name_len{property.name_len};
      memory::CopyMemory(&buffer[cursor], &property_name_len, kUsizeSize);
      cursor += kUsizeSize;

      memory::CopyMemory(&buffer[cursor], property.name, property_name_len);
      cursor += property_name_len;
    }
  }
}

void ShaderHandler::ParseDescr(const Array<u8>& dumped_descr,
                               memory::Allocator* allocator,
                               ShaderResourceDescr& descr) {
  constexpr auto kBoolSize{sizeof(bool)};
  constexpr auto kCullModeSize{sizeof(rendering::CullMode)};

  const auto* buffer{dumped_descr.GetData()};
  usize cursor{0};

  memory::CopyMemory(&descr.is_wireframe, &buffer[cursor], kBoolSize);
  cursor += kBoolSize;

  memory::CopyMemory(&descr.cull_mode, &buffer[cursor], kCullModeSize);
  cursor += kCullModeSize;

  ParseShaderModules(buffer, descr, cursor);
  ParseVertexAttributes(buffer, descr, cursor);
  ParseUniforms(buffer, descr, cursor);
  ParseConstants(buffer, descr, cursor);
  ParseStorages(buffer, allocator, descr, cursor);
}

void ShaderHandler::ParseShaderModules(const u8* buffer,
                                       ShaderResourceDescr& descr,
                                       usize& cursor) {
  constexpr auto kUsizeSize(sizeof(usize));

  usize module_path_count;
  memory::CopyMemory(&module_path_count, &buffer[cursor], kUsizeSize);
  cursor += kUsizeSize;

  descr.shader_module_paths.Reserve(module_path_count);

  for (usize i{0}; i < module_path_count; ++i) {
    usize module_path_size;
    memory::CopyMemory(&module_path_size, &buffer[cursor], kUsizeSize);
    cursor += kUsizeSize;

    const auto* str{reinterpret_cast<const tchar*>(&buffer[cursor])};

    TString module_path{str, str + module_path_size};
    cursor += module_path_size;

    descr.shader_module_paths.PushBack(std::move(module_path));
  }
}

void ShaderHandler::ParseVertexAttributes(const u8* buffer,
                                          ShaderResourceDescr& descr,
                                          usize& cursor) {
  constexpr auto kUsizeSize(sizeof(usize));
  constexpr auto kShaderVertexAttributeTypeSize(
      sizeof(rendering::ShaderVertexAttributeType));

  usize vertex_attribute_count;
  memory::CopyMemory(&vertex_attribute_count, &buffer[cursor], kUsizeSize);
  cursor += kUsizeSize;

  descr.vertex_attributes.Reserve(vertex_attribute_count);

  for (usize i{0}; i < vertex_attribute_count; ++i) {
    auto& vertex_attribute_descr{descr.vertex_attributes.EmplaceBack()};
    memory::CopyMemory(&vertex_attribute_descr.type, &buffer[cursor],
                       kShaderVertexAttributeTypeSize);
    cursor += kShaderVertexAttributeTypeSize;

    usize vertex_attribute_name_len;
    memory::CopyMemory(&vertex_attribute_name_len, &buffer[cursor], kUsizeSize);
    cursor += kUsizeSize;

    SetName(vertex_attribute_descr,
            reinterpret_cast<const schar*>(&buffer[cursor]),
            vertex_attribute_name_len);
    cursor += vertex_attribute_name_len;
  }
}

void ShaderHandler::ParseUniforms(const u8* buffer, ShaderResourceDescr& descr,
                                  usize& cursor) {
  constexpr auto kUsizeSize(sizeof(usize));
  constexpr auto kShaderUniformTypeSize(sizeof(rendering::ShaderVariableType));
  constexpr auto kShaderUniformScopeSize(sizeof(rendering::ShaderUniformScope));
  constexpr auto kShaderStageFlagsSize(sizeof(rendering::ShaderStageFlags));

  usize uniform_count;
  memory::CopyMemory(&uniform_count, &buffer[cursor], kUsizeSize);
  cursor += kUsizeSize;

  descr.uniforms.Reserve(uniform_count);

  for (usize i{0}; i < uniform_count; ++i) {
    auto& uniform_descr{descr.uniforms.EmplaceBack()};
    memory::CopyMemory(&uniform_descr.type, &buffer[cursor],
                       kShaderUniformTypeSize);
    cursor += kShaderUniformTypeSize;

    memory::CopyMemory(&uniform_descr.scope, &buffer[cursor],
                       kShaderUniformScopeSize);
    cursor += kShaderUniformScopeSize;

    memory::CopyMemory(&uniform_descr.stages, &buffer[cursor],
                       kShaderStageFlagsSize);
    cursor += kShaderStageFlagsSize;

    usize uniform_name_len;
    memory::CopyMemory(&uniform_name_len, &buffer[cursor], kUsizeSize);
    cursor += kUsizeSize;

    SetName(uniform_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
            uniform_name_len);
    cursor += uniform_name_len;
  }
}

void ShaderHandler::ParseConstants(const u8* buffer, ShaderResourceDescr& descr,
                                   usize& cursor) {
  constexpr auto kUsizeSize(sizeof(usize));
  constexpr auto kShaderConstantTypeSize(sizeof(rendering::ShaderVariableType));
  constexpr auto kShaderStageFlagsSize(sizeof(rendering::ShaderStageFlags));

  usize constant_count;
  memory::CopyMemory(&constant_count, &buffer[cursor], kUsizeSize);
  cursor += kUsizeSize;

  descr.constants.Reserve(constant_count);

  for (usize i{0}; i < constant_count; ++i) {
    auto& constant_descr{descr.constants.EmplaceBack()};
    memory::CopyMemory(&constant_descr.type, &buffer[cursor],
                       kShaderConstantTypeSize);
    cursor += kShaderConstantTypeSize;

    memory::CopyMemory(&constant_descr.stages, &buffer[cursor],
                       kShaderStageFlagsSize);
    cursor += kShaderStageFlagsSize;

    usize constant_name_len;
    memory::CopyMemory(&constant_name_len, &buffer[cursor], kUsizeSize);
    cursor += kUsizeSize;

    SetName(constant_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
            constant_name_len);
    cursor += constant_name_len;
  }
}

void ShaderHandler::ParseStorages(const u8* buffer,
                                  memory::Allocator* allocator,
                                  ShaderResourceDescr& descr, usize& cursor) {
  constexpr auto kUsizeSize(sizeof(usize));
  constexpr auto kShaderStageFlagsSize(sizeof(rendering::ShaderStageFlags));
  constexpr auto kShaderStoragePropertyTypeSize(
      sizeof(rendering::ShaderVariableType));

  usize storage_count;
  memory::CopyMemory(&storage_count, &buffer[cursor], kUsizeSize);
  cursor += kUsizeSize;

  descr.storages.Reserve(storage_count);

  for (usize i{0}; i < storage_count; ++i) {
    auto& storage_descr{descr.storages.EmplaceBack()};

    usize storage_name_len;
    memory::CopyMemory(&storage_name_len, &buffer[cursor], kUsizeSize);
    cursor += kUsizeSize;

    SetName(storage_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
            storage_name_len);
    cursor += storage_name_len;

    memory::CopyMemory(&storage_descr.stages, &buffer[cursor],
                       kShaderStageFlagsSize);
    cursor += kShaderStageFlagsSize;

    usize property_count;
    memory::CopyMemory(&property_count, &buffer[cursor], kUsizeSize);
    cursor += kUsizeSize;

    storage_descr.properties =
        Array<rendering::ShaderStoragePropertyDescr>{allocator};
    storage_descr.properties.Reserve(property_count);

    for (usize j{0}; j < property_count; ++j) {
      auto& property_descr{storage_descr.properties.EmplaceBack()};

      memory::CopyMemory(&property_descr.type, &buffer[cursor],
                         kShaderStoragePropertyTypeSize);
      cursor += kShaderStoragePropertyTypeSize;

      usize property_name_len;
      memory::CopyMemory(&property_name_len, &buffer[cursor], kUsizeSize);
      cursor += kUsizeSize;

      SetName(property_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
              property_name_len);
      cursor += property_name_len;
    }
  }
}
}  // namespace resource
}  // namespace comet
