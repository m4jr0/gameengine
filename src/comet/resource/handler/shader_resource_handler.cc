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
namespace internal {
constexpr usize kU32Size{sizeof(u32)};
constexpr usize kUsizeSize{sizeof(usize)};
constexpr usize kBoolSize{sizeof(bool)};
constexpr usize kCullModeSize{sizeof(rendering::CullMode)};
constexpr usize kCompareOpSize{sizeof(rendering::CompareOp)};
constexpr usize kPrimitiveTopologySize{sizeof(rendering::PrimitiveTopology)};
constexpr usize kShaderVertexLayoutSize{sizeof(rendering::ShaderVertexLayout)};
constexpr usize kShaderBindingTypeSize{sizeof(rendering::ShaderBindingType)};
constexpr usize kShaderBindingScopeSize{sizeof(rendering::ShaderBindingScope)};
constexpr usize kShaderMemoryLayoutSize{sizeof(rendering::ShaderMemoryLayout)};
constexpr usize kShaderStageFlagsSize{sizeof(rendering::ShaderStageFlags)};
constexpr usize kShaderVariableTypeSize{sizeof(rendering::ShaderVariableType)};
constexpr usize kShaderImageBindingSemanticSize{
    sizeof(rendering::ShaderImageBindingSemantic)};

usize GetTStringByteSize(const TString& str) {
  return str.GetLengthWithNullTerminator() * sizeof(tchar);
}
}  // namespace internal

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

  auto dumped_descr{DumpDescr(resource.descr)};
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
  auto data_size{GetSizeFromDescr(descr)};

  Array<u8> dumped_descr{byte_allocator_};
  dumped_descr.Resize(data_size);

  usize cursor{0};
  auto* buffer{dumped_descr.GetData()};

  memory::CopyMemory(&buffer[cursor], &descr.rasterizer.is_wireframe,
                     internal::kBoolSize);
  cursor += internal::kBoolSize;

  memory::CopyMemory(&buffer[cursor], &descr.rasterizer.is_depth_bias,
                     internal::kBoolSize);
  cursor += internal::kBoolSize;

  memory::CopyMemory(&buffer[cursor], &descr.rasterizer.cull_mode,
                     internal::kCullModeSize);
  cursor += internal::kCullModeSize;

  memory::CopyMemory(&buffer[cursor], &descr.depth_stencil.is_depth_test,
                     internal::kBoolSize);
  cursor += internal::kBoolSize;

  memory::CopyMemory(&buffer[cursor], &descr.depth_stencil.is_depth_write,
                     internal::kBoolSize);
  cursor += internal::kBoolSize;

  memory::CopyMemory(&buffer[cursor], &descr.depth_stencil.compare_op,
                     internal::kCompareOpSize);
  cursor += internal::kCompareOpSize;

  memory::CopyMemory(&buffer[cursor], &descr.topology,
                     internal::kPrimitiveTopologySize);
  cursor += internal::kPrimitiveTopologySize;

  memory::CopyMemory(&buffer[cursor], &descr.vertex_layout,
                     internal::kShaderVertexLayoutSize);
  cursor += internal::kShaderVertexLayoutSize;

  DumpShaderModules(descr, buffer, cursor);
  DumpShaderDefines(descr, buffer, cursor);
  DumpBindings(descr, buffer, cursor);
  DumpPushConstants(descr, buffer, cursor);

  return dumped_descr;
}

void ShaderResourceHandler::DumpShaderModules(const ShaderResourceDescr& descr,
                                              u8* buffer, usize& cursor) {
  auto module_path_count{descr.shader_module_paths.GetSize()};
  memory::CopyMemory(&buffer[cursor], &module_path_count, internal::kUsizeSize);
  cursor += internal::kUsizeSize;

  for (const auto& module_path : descr.shader_module_paths) {
    auto module_path_size{internal::GetTStringByteSize(module_path)};
    memory::CopyMemory(&buffer[cursor], &module_path_size,
                       internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    memory::CopyMemory(&buffer[cursor], module_path.GetCTStr(),
                       module_path_size);
    cursor += module_path_size;
  }
}

void ShaderResourceHandler::DumpShaderDefines(const ShaderResourceDescr& descr,
                                              u8* buffer, usize& cursor) {
  auto define_count{descr.defines.GetSize()};
  memory::CopyMemory(&buffer[cursor], &define_count, internal::kUsizeSize);
  cursor += internal::kUsizeSize;

  for (const auto& define : descr.defines) {
    auto define_name_len{define.name_len};
    memory::CopyMemory(&buffer[cursor], &define_name_len, internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    if (define_name_len > 0) {
      memory::CopyMemory(&buffer[cursor], define.name, define_name_len);
      cursor += define_name_len;
    }

    auto define_value_len{define.value_len};
    memory::CopyMemory(&buffer[cursor], &define_value_len,
                       internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    if (define_value_len > 0) {
      memory::CopyMemory(&buffer[cursor], define.value, define_value_len);
      cursor += define_value_len;
    }
  }
}

void ShaderResourceHandler::DumpBindings(const ShaderResourceDescr& descr,
                                         u8* buffer, usize& cursor) {
  auto binding_count{descr.bindings.GetSize()};
  memory::CopyMemory(&buffer[cursor], &binding_count, internal::kUsizeSize);
  cursor += internal::kUsizeSize;

  for (const auto& binding : descr.bindings) {
    auto binding_name_len{binding.name_len};
    memory::CopyMemory(&buffer[cursor], &binding_name_len,
                       internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    if (binding_name_len > 0) {
      memory::CopyMemory(&buffer[cursor], binding.name, binding_name_len);
      cursor += binding_name_len;
    }

    memory::CopyMemory(&buffer[cursor], &binding.type,
                       internal::kShaderBindingTypeSize);
    cursor += internal::kShaderBindingTypeSize;

    memory::CopyMemory(&buffer[cursor], &binding.scope,
                       internal::kShaderBindingScopeSize);
    cursor += internal::kShaderBindingScopeSize;

    memory::CopyMemory(&buffer[cursor], &binding.layout,
                       internal::kShaderMemoryLayoutSize);
    cursor += internal::kShaderMemoryLayoutSize;

    memory::CopyMemory(&buffer[cursor], &binding.set, internal::kU32Size);
    cursor += internal::kU32Size;

    memory::CopyMemory(&buffer[cursor], &binding.binding, internal::kU32Size);
    cursor += internal::kU32Size;

    memory::CopyMemory(&buffer[cursor], &binding.descriptor_count,
                       internal::kU32Size);
    cursor += internal::kU32Size;

    memory::CopyMemory(&buffer[cursor], &binding.image_semantic,
                       internal::kShaderImageBindingSemanticSize);
    cursor += internal::kShaderImageBindingSemanticSize;

    memory::CopyMemory(&buffer[cursor], &binding.stages,
                       internal::kShaderStageFlagsSize);
    cursor += internal::kShaderStageFlagsSize;

    auto field_count{binding.fields.GetSize()};
    memory::CopyMemory(&buffer[cursor], &field_count, internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    for (const auto& field : binding.fields) {
      auto field_name_len{field.name_len};
      memory::CopyMemory(&buffer[cursor], &field_name_len,
                         internal::kUsizeSize);
      cursor += internal::kUsizeSize;

      if (field_name_len > 0) {
        memory::CopyMemory(&buffer[cursor], field.name, field_name_len);
        cursor += field_name_len;
      }

      memory::CopyMemory(&buffer[cursor], &field.type,
                         internal::kShaderVariableTypeSize);
      cursor += internal::kShaderVariableTypeSize;

      memory::CopyMemory(&buffer[cursor], &field.array_count,
                         internal::kU32Size);
      cursor += internal::kU32Size;
    }
  }
}

void ShaderResourceHandler::DumpPushConstants(const ShaderResourceDescr& descr,
                                              u8* buffer, usize& cursor) {
  auto push_constant_count{descr.push_constants.GetSize()};
  memory::CopyMemory(&buffer[cursor], &push_constant_count,
                     internal::kUsizeSize);
  cursor += internal::kUsizeSize;

  for (const auto& push_constant : descr.push_constants) {
    auto push_constant_name_len{push_constant.name_len};
    memory::CopyMemory(&buffer[cursor], &push_constant_name_len,
                       internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    if (push_constant_name_len > 0) {
      memory::CopyMemory(&buffer[cursor], push_constant.name,
                         push_constant_name_len);
      cursor += push_constant_name_len;
    }

    memory::CopyMemory(&buffer[cursor], &push_constant.stages,
                       internal::kShaderStageFlagsSize);
    cursor += internal::kShaderStageFlagsSize;

    auto field_count{push_constant.fields.GetSize()};
    memory::CopyMemory(&buffer[cursor], &field_count, internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    for (const auto& field : push_constant.fields) {
      auto field_name_len{field.name_len};
      memory::CopyMemory(&buffer[cursor], &field_name_len,
                         internal::kUsizeSize);
      cursor += internal::kUsizeSize;

      if (field_name_len > 0) {
        memory::CopyMemory(&buffer[cursor], field.name, field_name_len);
        cursor += field_name_len;
      }

      memory::CopyMemory(&buffer[cursor], &field.type,
                         internal::kShaderVariableTypeSize);
      cursor += internal::kShaderVariableTypeSize;

      memory::CopyMemory(&buffer[cursor], &field.array_count,
                         internal::kU32Size);
      cursor += internal::kU32Size;
    }
  }
}

void ShaderResourceHandler::ParseDescr(const Array<u8>& dumped_descr,
                                       ShaderResourceDescr& descr) {
  descr.shader_module_paths = Array<TString>{byte_allocator_};
  descr.defines = Array<rendering::ShaderDefineDescr>{byte_allocator_};
  descr.bindings = Array<rendering::ShaderBindingDescr>{byte_allocator_};
  descr.push_constants =
      Array<rendering::ShaderPushConstantDescr>{byte_allocator_};

  const auto* buffer{dumped_descr.GetData()};
  usize cursor{0};

  memory::CopyMemory(&descr.rasterizer.is_wireframe, &buffer[cursor],
                     internal::kBoolSize);
  cursor += internal::kBoolSize;

  memory::CopyMemory(&descr.rasterizer.is_depth_bias, &buffer[cursor],
                     internal::kBoolSize);
  cursor += internal::kBoolSize;

  memory::CopyMemory(&descr.rasterizer.cull_mode, &buffer[cursor],
                     internal::kCullModeSize);
  cursor += internal::kCullModeSize;

  memory::CopyMemory(&descr.depth_stencil.is_depth_test, &buffer[cursor],
                     internal::kBoolSize);
  cursor += internal::kBoolSize;

  memory::CopyMemory(&descr.depth_stencil.is_depth_write, &buffer[cursor],
                     internal::kBoolSize);
  cursor += internal::kBoolSize;

  memory::CopyMemory(&descr.depth_stencil.compare_op, &buffer[cursor],
                     internal::kCompareOpSize);
  cursor += internal::kCompareOpSize;

  memory::CopyMemory(&descr.topology, &buffer[cursor],
                     internal::kPrimitiveTopologySize);
  cursor += internal::kPrimitiveTopologySize;

  memory::CopyMemory(&descr.vertex_layout, &buffer[cursor],
                     internal::kShaderVertexLayoutSize);
  cursor += internal::kShaderVertexLayoutSize;

  ParseShaderModules(buffer, descr, cursor);
  ParseShaderDefines(buffer, descr, cursor);
  ParseBindings(buffer, descr, cursor);
  ParsePushConstants(buffer, descr, cursor);
}

void ShaderResourceHandler::ParseShaderModules(const u8* buffer,
                                               ShaderResourceDescr& descr,
                                               usize& cursor) {
  usize module_path_count{0};
  memory::CopyMemory(&module_path_count, &buffer[cursor], internal::kUsizeSize);
  cursor += internal::kUsizeSize;

  descr.shader_module_paths.Reserve(module_path_count);

  for (usize i{0}; i < module_path_count; ++i) {
    usize module_path_size{0};
    memory::CopyMemory(&module_path_size, &buffer[cursor],
                       internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    const auto* str{reinterpret_cast<const tchar*>(&buffer[cursor])};
    auto char_count{module_path_size / sizeof(tchar)};

    TString module_path{str, str + char_count - 1};
    cursor += module_path_size;

    descr.shader_module_paths.PushBack(std::move(module_path));
  }
}

void ShaderResourceHandler::ParseShaderDefines(const u8* buffer,
                                               ShaderResourceDescr& descr,
                                               usize& cursor) {
  usize define_count{0};
  memory::CopyMemory(&define_count, &buffer[cursor], internal::kUsizeSize);
  cursor += internal::kUsizeSize;

  descr.defines.Reserve(define_count);

  for (usize i{0}; i < define_count; ++i) {
    auto& define_descr{descr.defines.EmplaceBack()};

    usize define_name_len{0};
    memory::CopyMemory(&define_name_len, &buffer[cursor], internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    if (define_name_len > 0) {
      SetName(define_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
              define_name_len);
      cursor += define_name_len;
    }

    usize define_value_len{0};
    memory::CopyMemory(&define_value_len, &buffer[cursor],
                       internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    if (define_value_len > 0) {
      SetValue(define_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
               define_value_len);
      cursor += define_value_len;
    }
  }
}

void ShaderResourceHandler::ParseBindings(const u8* buffer,
                                          ShaderResourceDescr& descr,
                                          usize& cursor) {
  usize binding_count{0};
  memory::CopyMemory(&binding_count, &buffer[cursor], internal::kUsizeSize);
  cursor += internal::kUsizeSize;

  descr.bindings.Reserve(binding_count);

  for (usize i{0}; i < binding_count; ++i) {
    auto& binding_descr{descr.bindings.EmplaceBack()};

    usize binding_name_len{0};
    memory::CopyMemory(&binding_name_len, &buffer[cursor],
                       internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    if (binding_name_len > 0) {
      SetName(binding_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
              binding_name_len);
      cursor += binding_name_len;
    }

    memory::CopyMemory(&binding_descr.type, &buffer[cursor],
                       internal::kShaderBindingTypeSize);
    cursor += internal::kShaderBindingTypeSize;

    memory::CopyMemory(&binding_descr.scope, &buffer[cursor],
                       internal::kShaderBindingScopeSize);
    cursor += internal::kShaderBindingScopeSize;

    memory::CopyMemory(&binding_descr.layout, &buffer[cursor],
                       internal::kShaderMemoryLayoutSize);
    cursor += internal::kShaderMemoryLayoutSize;

    memory::CopyMemory(&binding_descr.set, &buffer[cursor], internal::kU32Size);
    cursor += internal::kU32Size;

    memory::CopyMemory(&binding_descr.binding, &buffer[cursor],
                       internal::kU32Size);
    cursor += internal::kU32Size;

    memory::CopyMemory(&binding_descr.descriptor_count, &buffer[cursor],
                       internal::kU32Size);
    cursor += internal::kU32Size;

    memory::CopyMemory(&binding_descr.image_semantic, &buffer[cursor],
                       internal::kShaderImageBindingSemanticSize);
    cursor += internal::kShaderImageBindingSemanticSize;

    memory::CopyMemory(&binding_descr.stages, &buffer[cursor],
                       internal::kShaderStageFlagsSize);
    cursor += internal::kShaderStageFlagsSize;

    usize field_count{0};
    memory::CopyMemory(&field_count, &buffer[cursor], internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    binding_descr.fields = Array<rendering::ShaderFieldDescr>{byte_allocator_};
    binding_descr.fields.Reserve(field_count);

    for (usize j{0}; j < field_count; ++j) {
      auto& field_descr{binding_descr.fields.EmplaceBack()};

      usize field_name_len{0};
      memory::CopyMemory(&field_name_len, &buffer[cursor],
                         internal::kUsizeSize);
      cursor += internal::kUsizeSize;

      if (field_name_len > 0) {
        SetName(field_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
                field_name_len);
        cursor += field_name_len;
      }

      memory::CopyMemory(&field_descr.type, &buffer[cursor],
                         internal::kShaderVariableTypeSize);
      cursor += internal::kShaderVariableTypeSize;

      memory::CopyMemory(&field_descr.array_count, &buffer[cursor],
                         internal::kU32Size);
      cursor += internal::kU32Size;
    }
  }
}

void ShaderResourceHandler::ParsePushConstants(const u8* buffer,
                                               ShaderResourceDescr& descr,
                                               usize& cursor) {
  usize push_constant_count{0};
  memory::CopyMemory(&push_constant_count, &buffer[cursor],
                     internal::kUsizeSize);
  cursor += internal::kUsizeSize;

  descr.push_constants.Reserve(push_constant_count);

  for (usize i{0}; i < push_constant_count; ++i) {
    auto& push_constant_descr{descr.push_constants.EmplaceBack()};

    usize push_constant_name_len{0};
    memory::CopyMemory(&push_constant_name_len, &buffer[cursor],
                       internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    if (push_constant_name_len > 0) {
      SetName(push_constant_descr,
              reinterpret_cast<const schar*>(&buffer[cursor]),
              push_constant_name_len);
      cursor += push_constant_name_len;
    }

    memory::CopyMemory(&push_constant_descr.stages, &buffer[cursor],
                       internal::kShaderStageFlagsSize);
    cursor += internal::kShaderStageFlagsSize;

    usize field_count{0};
    memory::CopyMemory(&field_count, &buffer[cursor], internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    push_constant_descr.fields =
        Array<rendering::ShaderFieldDescr>{byte_allocator_};
    push_constant_descr.fields.Reserve(field_count);

    for (usize j{0}; j < field_count; ++j) {
      auto& field_descr{push_constant_descr.fields.EmplaceBack()};

      usize field_name_len{0};
      memory::CopyMemory(&field_name_len, &buffer[cursor],
                         internal::kUsizeSize);
      cursor += internal::kUsizeSize;

      if (field_name_len > 0) {
        SetName(field_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
                field_name_len);
        cursor += field_name_len;
      }

      memory::CopyMemory(&field_descr.type, &buffer[cursor],
                         internal::kShaderVariableTypeSize);
      cursor += internal::kShaderVariableTypeSize;

      memory::CopyMemory(&field_descr.array_count, &buffer[cursor],
                         internal::kU32Size);
      cursor += internal::kU32Size;
    }
  }
}
}  // namespace resource
}  // namespace comet