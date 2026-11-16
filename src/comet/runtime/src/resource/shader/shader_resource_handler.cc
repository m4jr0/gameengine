// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/shader/shader_resource_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/container/array.h"
#include "comet/core/string/tstring.h"
#include "comet/data/render/pipeline.h"
#include "comet/data/render/shader.h"
#include "comet/data/render/utils/shader_utils.h"

namespace comet {
namespace resource {
namespace internal {
constexpr usize kU32Size{sizeof(u32)};
constexpr usize kUsizeSize{sizeof(usize)};
constexpr usize kBoolSize{sizeof(bool)};
constexpr usize kCullModeSize{sizeof(render::CullMode)};
constexpr usize kCompareOpSize{sizeof(render::CompareOp)};
constexpr usize kPrimitiveTopologySize{sizeof(render::PrimitiveTopology)};
constexpr usize kShaderVertexLayoutSize{sizeof(render::ShaderVertexLayout)};
constexpr usize kShaderBindingTypeSize{sizeof(render::ShaderBindingType)};
constexpr usize kShaderBindingScopeSize{sizeof(render::ShaderBindingScope)};
constexpr usize kShaderMemoryLayoutSize{sizeof(render::ShaderMemoryLayout)};
constexpr usize kShaderStageFlagsSize{sizeof(render::ShaderStageFlags)};
constexpr usize kShaderVariableTypeSize{sizeof(render::ShaderVariableType)};
constexpr usize kShaderImageBindingSemanticSize{
    sizeof(render::ShaderImageBindingSemantic)};

usize GetTStringByteSize(const TString& str) {
  return str.GetLengthWithNullTerminator() * sizeof(tchar);
}
}  // namespace internal

ShaderResourceHandler::ShaderResourceHandler(const ResourceHandlerDescr& descr)
    : Base{descr} {}

ResourceFile ShaderResourceHandler::Pack(const ShaderResource& resource,
                                         CompressionMode compression_mode) {
  return PackShaderResource(resource, compression_mode, byte_allocator_);
}

void ShaderResourceHandler::Unpack(const ResourceFile& file,
                                   ResourceLifeSpan life_span,
                                   ShaderResource* resource) {
  COMET_ASSERT(resource != nullptr, "ShaderResourceHandler::Unpack",
               "shader resource is null");

  Array<u8> dumped_descr{byte_allocator_};
  UnpackBytes(file.compression_mode, file.descr, file.descr_size, dumped_descr);
  ParseDescr(dumped_descr, resource->descr);

  Array<u8> data{byte_allocator_};
  UnpackResourceData(file, data);

  const auto* buffer{data.GetData()};
  usize cursor{0};

  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};

  COMET_ASSERT(data.GetSize() >= kResourceIdSize + kResourceTypeIdSize,
               "ShaderResourceHandler::Unpack",
               "shader resource data is too small", "data_size",
               data.GetSize());

  memory::CopyMemory(&resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&resource->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;
  COMET_ASSERT(resource->type_id == ShaderResource::kResourceTypeId,
               "ShaderResourceHandler::Unpack",
               "shader resource type id is invalid", "resource_type_id",
               resource->type_id, "expected_type_id",
               ShaderResource::kResourceTypeId);

  COMET_ASSERT(cursor == data.GetSize(), "ShaderResourceHandler::Unpack",
               "unpacked shader resource size mismatch", "cursor", cursor,
               "data_size", data.GetSize());

  resource->life_span = life_span;
}

void ShaderResourceHandler::ParseDescr(const Array<u8>& dumped_descr,
                                       ShaderResourceDescr& descr) {
  descr.shader_module_resource_ids =
      Array<ShaderModuleResourceId>{byte_allocator_};
  descr.defines = Array<render::ShaderDefineDescr>{byte_allocator_};
  descr.bindings = Array<render::ShaderBindingDescr>{byte_allocator_};
  descr.push_constants =
      Array<render::ShaderPushConstantDescr>{byte_allocator_};

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

  COMET_ASSERT(cursor == dumped_descr.GetSize(),
               "ShaderResourceHandler::ParseDescr",
               "parsed shader description size mismatch", "cursor", cursor,
               "descr_size", dumped_descr.GetSize());
}

void ShaderResourceHandler::ParseShaderModules(const u8* buffer,
                                               ShaderResourceDescr& descr,
                                               usize& cursor) {
  usize module_count{0};
  memory::CopyMemory(&module_count, &buffer[cursor], internal::kUsizeSize);
  cursor += internal::kUsizeSize;

  descr.shader_module_resource_ids.Reserve(module_count);

  constexpr auto kShaderModuleResourceIdSize{sizeof(ShaderModuleResourceId)};

  for (usize i{0}; i < module_count; ++i) {
    auto& module_resource_id{descr.shader_module_resource_ids.EmplaceLast()};
    memory::CopyMemory(&module_resource_id, &buffer[cursor],
                       kShaderModuleResourceIdSize);
    cursor += kShaderModuleResourceIdSize;
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
    auto& define_descr{descr.defines.EmplaceLast()};

    usize define_name_len{0};
    memory::CopyMemory(&define_name_len, &buffer[cursor], internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    if (define_name_len > 0) {
      COMET_ASSERT(define_name_len < render::kMaxShaderDefineNameLen + 1,
                   "ShaderResourceHandler::ParseShaderDefines",
                   "shader define name length is invalid", "name_len",
                   define_name_len);
      SetName(define_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
              define_name_len);
      cursor += define_name_len;
    }

    usize define_value_len{0};
    memory::CopyMemory(&define_value_len, &buffer[cursor],
                       internal::kUsizeSize);
    cursor += internal::kUsizeSize;

    if (define_value_len > 0) {
      COMET_ASSERT(define_value_len < render::kMaxShaderDefineValueLen + 1,
                   "ShaderResourceHandler::ParseShaderDefines",
                   "shader define value length is invalid", "value_len",
                   define_value_len);
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
    auto& binding_descr{descr.bindings.EmplaceLast()};

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

    binding_descr.fields = Array<render::ShaderFieldDescr>{byte_allocator_};
    binding_descr.fields.Reserve(field_count);

    for (usize j{0}; j < field_count; ++j) {
      auto& field_descr{binding_descr.fields.EmplaceLast()};

      usize field_name_len{0};
      memory::CopyMemory(&field_name_len, &buffer[cursor],
                         internal::kUsizeSize);
      cursor += internal::kUsizeSize;

      if (field_name_len > 0) {
        COMET_ASSERT(field_name_len < render::kShaderNameMaxLen + 1,
                     "ShaderResourceHandler::ParseBindings",
                     "shader field name length is invalid", "name_len",
                     field_name_len);
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
    auto& push_constant_descr{descr.push_constants.EmplaceLast()};

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
        Array<render::ShaderFieldDescr>{byte_allocator_};
    push_constant_descr.fields.Reserve(field_count);

    for (usize j{0}; j < field_count; ++j) {
      auto& field_descr{push_constant_descr.fields.EmplaceLast()};

      usize field_name_len{0};
      memory::CopyMemory(&field_name_len, &buffer[cursor],
                         internal::kUsizeSize);
      cursor += internal::kUsizeSize;

      if (field_name_len > 0) {
        COMET_ASSERT(field_name_len < render::kShaderNameMaxLen + 1,
                     "ShaderResourceHandler::ParseBindings",
                     "shader field name length is invalid", "name_len",
                     field_name_len);
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