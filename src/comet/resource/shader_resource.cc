// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "shader_resource.h"

#include "comet/core/c_string.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace resource {
const ResourceTypeId ShaderResource::kResourceTypeId{COMET_STRING_ID("shader")};

ResourceFile ShaderHandler::Pack(const Resource& resource,
                                 CompressionMode compression_mode) const {
  const auto& shader{static_cast<const ShaderResource&>(resource)};
  ResourceFile file{};
  file.resource_id = shader.id;
  file.resource_type_id = ShaderResource::kResourceTypeId;
  file.compression_mode = compression_mode;

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  std::vector<u8> data(kResourceIdSize + kResourceTypeIdSize);
  uindex cursor{0};
  auto* buffer{data.data()};

  CopyMemory(&buffer[cursor], &shader.id, kResourceIdSize);
  cursor += kResourceIdSize;

  CopyMemory(&buffer[cursor], &shader.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  const auto dumped_descr{DumpDescr(shader.descr)};
  file.descr_size = dumped_descr.size();
  PackBytes(dumped_descr, file.compression_mode, &file.descr,
            &file.packed_descr_size);
  PackResourceData(data, file);
  return file;
}

std::unique_ptr<Resource> ShaderHandler::Unpack(
    const ResourceFile& file) const {
  ShaderResource shader{};
  auto dumpedDescr{
      UnpackBytes(file.compression_mode, file.descr, file.descr_size)};
  shader.descr = ParseDescr(dumpedDescr);

  const auto data{UnpackResourceData(file)};
  const auto* buffer{data.data()};
  uindex cursor{0};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};

  CopyMemory(&shader.id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  CopyMemory(&shader.type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  return std::make_unique<ShaderResource>(std::move(shader));
}

std::vector<u8> ShaderHandler::DumpDescr(
    const ShaderResourceDescr& descr) const {
  constexpr auto kBoolSize{sizeof(bool)};
  constexpr auto kCullModeSize{sizeof(rendering::CullMode)};
  constexpr auto kUIndexSize(sizeof(uindex));
  constexpr auto kShaderVertexAttributeTypeSize(
      sizeof(rendering::ShaderVertexAttributeType));
  constexpr auto kShaderUniformTypeSize(sizeof(rendering::ShaderUniformType));
  constexpr auto kShaderUniformScopeSize(sizeof(rendering::ShaderUniformScope));

  // is_wireframe size, cull_mode size, and shader_module_paths size.
  auto total_size{kBoolSize + kCullModeSize + kUIndexSize};

  for (const auto& module_path : descr.shader_module_paths) {
    total_size += kUIndexSize +
                  (module_path.GetLengthWithNullTerminator()) * sizeof(tchar);
  }

  total_size += kUIndexSize;

  for (const auto& vertex_attribute : descr.vertex_attributes) {
    total_size += kShaderVertexAttributeTypeSize + kUIndexSize +
                  vertex_attribute.name_len;
  }

  total_size += kUIndexSize;

  for (const auto& uniform : descr.uniforms) {
    total_size += kShaderUniformTypeSize + kShaderUniformScopeSize +
                  kUIndexSize + uniform.name_len;
  }

  const auto data_size{total_size};
  std::vector<u8> dumped_descr{};
  dumped_descr.resize(data_size);
  uindex cursor{0};
  auto* buffer{dumped_descr.data()};

  CopyMemory(&buffer[cursor], &descr.is_wireframe, kBoolSize);
  cursor += kBoolSize;

  CopyMemory(&buffer[cursor], &descr.cull_mode, kCullModeSize);
  cursor += kCullModeSize;

  const auto module_path_count{descr.shader_module_paths.size()};
  CopyMemory(&buffer[cursor], &module_path_count, kUIndexSize);
  cursor += kUIndexSize;

  for (const auto& module_path : descr.shader_module_paths) {
    const auto module_path_size{(module_path.GetLengthWithNullTerminator()) *
                                sizeof(tchar)};
    CopyMemory(&buffer[cursor], &module_path_size, kUIndexSize);
    cursor += kUIndexSize;

    CopyMemory(&buffer[cursor], module_path.GetCTStr(), module_path_size);
    cursor += module_path_size;
  }

  const auto vertex_attribute_count{descr.vertex_attributes.size()};
  CopyMemory(&buffer[cursor], &vertex_attribute_count, kUIndexSize);
  cursor += kUIndexSize;

  for (const auto& vertex_attribute : descr.vertex_attributes) {
    CopyMemory(&buffer[cursor], &vertex_attribute.type,
               kShaderVertexAttributeTypeSize);
    cursor += kShaderVertexAttributeTypeSize;

    const auto vertex_attribute_name_size{vertex_attribute.name_len};
    CopyMemory(&buffer[cursor], &vertex_attribute_name_size, kUIndexSize);
    cursor += kUIndexSize;

    CopyMemory(&buffer[cursor], vertex_attribute.name,
               vertex_attribute_name_size);
    cursor += vertex_attribute_name_size;
  }

  const auto uniform_count{descr.uniforms.size()};
  CopyMemory(&buffer[cursor], &uniform_count, kUIndexSize);
  cursor += kUIndexSize;

  for (const auto& uniform : descr.uniforms) {
    CopyMemory(&buffer[cursor], &uniform.type, kShaderUniformTypeSize);
    cursor += kShaderUniformTypeSize;

    CopyMemory(&buffer[cursor], &uniform.scope, kShaderUniformScopeSize);
    cursor += kShaderUniformScopeSize;

    const auto uniform_name_size{uniform.name_len};
    CopyMemory(&buffer[cursor], &uniform_name_size, kUIndexSize);
    cursor += kUIndexSize;

    CopyMemory(&buffer[cursor], uniform.name, uniform_name_size);
    cursor += uniform_name_size;
  }

  return dumped_descr;
}

ShaderResourceDescr ShaderHandler::ParseDescr(
    const std::vector<u8>& dumped_descr) const {
  constexpr auto kBoolSize{sizeof(bool)};
  constexpr auto kCullModeSize{sizeof(rendering::CullMode)};
  constexpr auto kUIndexSize(sizeof(uindex));
  constexpr auto kShaderVertexAttributeTypeSize(
      sizeof(rendering::ShaderVertexAttributeType));
  constexpr auto kShaderUniformTypeSize(sizeof(rendering::ShaderUniformType));
  constexpr auto kShaderUniformScopeSize(sizeof(rendering::ShaderUniformScope));

  const auto* buffer{dumped_descr.data()};
  uindex cursor{0};
  ShaderResourceDescr descr{};

  CopyMemory(&descr.is_wireframe, &buffer[cursor], kBoolSize);
  cursor += kBoolSize;

  CopyMemory(&descr.cull_mode, &buffer[cursor], kCullModeSize);
  cursor += kCullModeSize;

  uindex module_path_count;
  CopyMemory(&module_path_count, &buffer[cursor], kUIndexSize);
  cursor += kUIndexSize;

  descr.shader_module_paths.reserve(module_path_count);

  for (uindex i{0}; i < module_path_count; ++i) {
    uindex module_path_size;
    CopyMemory(&module_path_size, &buffer[cursor], kUIndexSize);
    cursor += kUIndexSize;

    const auto* str{reinterpret_cast<const tchar*>(&buffer[cursor])};

    TString module_path{str, str + module_path_size};
    cursor += module_path_size;

    descr.shader_module_paths.push_back(std::move(module_path));
  }

  uindex vertex_attribute_count;
  CopyMemory(&vertex_attribute_count, &buffer[cursor], kUIndexSize);
  cursor += kUIndexSize;

  descr.vertex_attributes.reserve(vertex_attribute_count);

  for (uindex i{0}; i < vertex_attribute_count; ++i) {
    rendering::ShaderVertexAttributeDescr vertex_attribute_descr{};

    CopyMemory(&vertex_attribute_descr.type, &buffer[cursor],
               kShaderVertexAttributeTypeSize);
    cursor += kShaderVertexAttributeTypeSize;

    uindex vertex_attribute_name_len;
    CopyMemory(&vertex_attribute_name_len, &buffer[cursor], kUIndexSize);
    cursor += kUIndexSize;

    SetName(vertex_attribute_descr,
            reinterpret_cast<const schar*>(&buffer[cursor]),
            vertex_attribute_name_len);
    cursor += vertex_attribute_name_len;

    descr.vertex_attributes.push_back(std::move(vertex_attribute_descr));
  }

  uindex uniform_count;
  CopyMemory(&uniform_count, &buffer[cursor], kUIndexSize);
  cursor += kUIndexSize;

  descr.uniforms.reserve(uniform_count);

  for (uindex i{0}; i < uniform_count; ++i) {
    rendering::ShaderUniformDescr uniform_descr{};

    CopyMemory(&uniform_descr.type, &buffer[cursor], kShaderUniformTypeSize);
    cursor += kShaderUniformTypeSize;

    CopyMemory(&uniform_descr.scope, &buffer[cursor], kShaderUniformScopeSize);
    cursor += kShaderUniformScopeSize;

    uindex uniform_name_len;
    CopyMemory(&uniform_name_len, &buffer[cursor], kUIndexSize);
    cursor += kUIndexSize;

    SetName(uniform_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
            uniform_name_len);
    cursor += uniform_name_len;

    descr.uniforms.push_back(std::move(uniform_descr));
  }

  return descr;
}
}  // namespace resource
}  // namespace comet
