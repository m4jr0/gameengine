// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "shader_resource_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/core/type/tstring.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace resource {
ShaderResourceHandler::ShaderResourceHandler(const ResourceHandlerDescr& descr)
    : ResourceHandler<ShaderResource>{descr} {}

ResourceFile ShaderResourceHandler::Pack(const ShaderResource& resource,
                                         CompressionMode compression_mode) {
  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = ShaderResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{byte_allocator_};
  file.data = Array<u8>{byte_allocator_};

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  Array<u8> data{byte_allocator_};
  data.Resize(kResourceIdSize + kResourceTypeIdSize);
  usize cursor{0};
  auto* buffer{data.GetData()};

  memory::CopyMemory(&buffer[cursor], &resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  const auto dumped_descr{DumpDescr(resource.descr)};
  file.descr_size = dumped_descr.GetSize();
  PackBytes(dumped_descr, file.compression_mode, &file.descr,
            &file.packed_descr_size);
  PackResourceData(data, file);
  return file;
}

void ShaderResourceHandler::Unpack(const ResourceFile& file,
                                   ResourceLifeSpan life_span,
                                   ShaderResource* resource) {
  Array<u8> dumped_descr{byte_allocator_};
  UnpackBytes(file.compression_mode, file.descr, file.descr_size, dumped_descr);
  ParseDescr(dumped_descr, resource->descr);

  Array<u8> data{byte_allocator_};
  UnpackResourceData(file, data);
  const auto* buffer{data.GetData()};
  usize cursor{0};

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};

  memory::CopyMemory(&resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&resource->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;
  resource->life_span = life_span;
}

Array<u8> ShaderResourceHandler::DumpDescr(const ShaderResourceDescr& descr) {
  constexpr auto kBoolSize{sizeof(bool)};
  constexpr auto kCullModeSize{sizeof(rendering::CullMode)};
  constexpr auto kPrimitiveTopologySize{sizeof(rendering::PrimitiveTopology)};

  const auto data_size{GetSizeFromDescr(descr)};
  Array<u8> dumped_descr{byte_allocator_};
  dumped_descr.Resize(data_size);
  usize cursor{0};
  auto* buffer{dumped_descr.GetData()};

  memory::CopyMemory(&buffer[cursor], &descr.is_wireframe, kBoolSize);
  cursor += kBoolSize;

  memory::CopyMemory(&buffer[cursor], &descr.cull_mode, kCullModeSize);
  cursor += kCullModeSize;

  memory::CopyMemory(&buffer[cursor], &descr.topology, kPrimitiveTopologySize);
  cursor += kPrimitiveTopologySize;

  DumpShaderModules(descr, buffer, cursor);
  DumpShaderDefines(descr, buffer, cursor);
  DumpVertexAttributes(descr, buffer, cursor);
  DumpUniforms(descr, buffer, cursor);
  DumpConstants(descr, buffer, cursor);
  DumpStorages(descr, buffer, cursor);

  return dumped_descr;
}

void ShaderResourceHandler::DumpShaderModules(const ShaderResourceDescr& descr,
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

void ShaderResourceHandler::DumpShaderDefines(const ShaderResourceDescr& descr,
                                              u8* buffer, usize& cursor) {
  constexpr auto kUsizeSize(sizeof(usize));

  const auto define_count{descr.defines.GetSize()};
  memory::CopyMemory(&buffer[cursor], &define_count, kUsizeSize);
  cursor += kUsizeSize;

  for (const auto& define : descr.defines) {
    const auto define_name_len{define.name_len};
    memory::CopyMemory(&buffer[cursor], &define_name_len, kUsizeSize);
    cursor += kUsizeSize;

    memory::CopyMemory(&buffer[cursor], define.name, define_name_len);
    cursor += define_name_len;

    const auto define_value_len{define.value_len};
    memory::CopyMemory(&buffer[cursor], &define_value_len, kUsizeSize);
    cursor += kUsizeSize;

    memory::CopyMemory(&buffer[cursor], define.value, define_value_len);
    cursor += define_value_len;
  }
}

void ShaderResourceHandler::DumpVertexAttributes(
    const ShaderResourceDescr& descr, u8* buffer, usize& cursor) {
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

void ShaderResourceHandler::DumpUniforms(const ShaderResourceDescr& descr,
                                         u8* buffer, usize& cursor) {
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

void ShaderResourceHandler::DumpConstants(const ShaderResourceDescr& descr,
                                          u8* buffer, usize& cursor) {
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

void ShaderResourceHandler::DumpStorages(const ShaderResourceDescr& descr,
                                         u8* buffer, usize& cursor) {
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

    const auto storage_engine_define_len{storage.engine_define_len};
    memory::CopyMemory(&buffer[cursor], &storage_engine_define_len, kUsizeSize);
    cursor += kUsizeSize;

    memory::CopyMemory(&buffer[cursor], storage.engine_define,
                       storage_engine_define_len);
    cursor += storage_engine_define_len;
  }
}

void ShaderResourceHandler::ParseDescr(const Array<u8>& dumped_descr,
                                       ShaderResourceDescr& descr) {
  constexpr auto kBoolSize{sizeof(bool)};
  constexpr auto kCullModeSize{sizeof(rendering::CullMode)};
  constexpr auto kPrimitiveTopologySize{sizeof(rendering::PrimitiveTopology)};

  descr.shader_module_paths = Array<TString>{byte_allocator_};
  descr.defines = Array<rendering::ShaderDefineDescr>{byte_allocator_};
  descr.vertex_attributes =
      Array<rendering::ShaderVertexAttributeDescr>{byte_allocator_};
  descr.uniforms = Array<rendering::ShaderUniformDescr>{byte_allocator_};
  descr.constants = Array<rendering::ShaderConstantDescr>{byte_allocator_};
  descr.storages = Array<rendering::ShaderStorageDescr>{byte_allocator_};

  const auto* buffer{dumped_descr.GetData()};
  usize cursor{0};

  memory::CopyMemory(&descr.is_wireframe, &buffer[cursor], kBoolSize);
  cursor += kBoolSize;

  memory::CopyMemory(&descr.cull_mode, &buffer[cursor], kCullModeSize);
  cursor += kCullModeSize;

  memory::CopyMemory(&descr.topology, &buffer[cursor], kPrimitiveTopologySize);
  cursor += kPrimitiveTopologySize;

  ParseShaderModules(buffer, descr, cursor);
  ParseShaderDefines(buffer, descr, cursor);
  ParseVertexAttributes(buffer, descr, cursor);
  ParseUniforms(buffer, descr, cursor);
  ParseConstants(buffer, descr, cursor);
  ParseStorages(buffer, descr, cursor);
}

void ShaderResourceHandler::ParseShaderModules(const u8* buffer,
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

void ShaderResourceHandler::ParseShaderDefines(const u8* buffer,
                                               ShaderResourceDescr& descr,
                                               usize& cursor) {
  constexpr auto kUsizeSize(sizeof(usize));

  usize define_count;
  memory::CopyMemory(&define_count, &buffer[cursor], kUsizeSize);
  cursor += kUsizeSize;

  descr.defines.Reserve(define_count);

  for (usize j{0}; j < define_count; ++j) {
    auto& define_descr{descr.defines.EmplaceBack()};

    usize define_name_len;
    memory::CopyMemory(&define_name_len, &buffer[cursor], kUsizeSize);
    cursor += kUsizeSize;

    SetName(define_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
            define_name_len);
    cursor += define_name_len;

    usize define_value_len;
    memory::CopyMemory(&define_value_len, &buffer[cursor], kUsizeSize);
    cursor += kUsizeSize;

    if (define_value_len > 0) {
      SetValue(define_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
               define_value_len);
      cursor += define_value_len;
    }
  }
}

void ShaderResourceHandler::ParseVertexAttributes(const u8* buffer,
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

void ShaderResourceHandler::ParseUniforms(const u8* buffer,
                                          ShaderResourceDescr& descr,
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

void ShaderResourceHandler::ParseConstants(const u8* buffer,
                                           ShaderResourceDescr& descr,
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

void ShaderResourceHandler::ParseStorages(const u8* buffer,
                                          ShaderResourceDescr& descr,
                                          usize& cursor) {
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
        Array<rendering::ShaderStoragePropertyDescr>{byte_allocator_};
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

    usize engine_define_len;
    memory::CopyMemory(&engine_define_len, &buffer[cursor], kUsizeSize);
    cursor += kUsizeSize;

    SetEngineDefine(storage_descr,
                    reinterpret_cast<const schar*>(&buffer[cursor]),
                    engine_define_len);
    cursor += engine_define_len;
  }
}
}  // namespace resource
}  // namespace comet
