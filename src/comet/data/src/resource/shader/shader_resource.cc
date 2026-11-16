// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/resource/shader/shader_resource.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/string/c_string.h"
#include "comet/core/id/string_id.h"
#include "comet/core/id/string_id_allocator.h"
#include "comet/core/memory/memory_utils.h"

namespace comet {
namespace resource {
const ShaderResource::TypeId ShaderResource::kResourceTypeId{
    COMET_STRING_ID(ShaderResource::kResourceTypeName.data())};

ShaderResource::Id ShaderResource::GetId() const noexcept { return Id{id}; }

usize GetSizeFromDescr(const ShaderResourceDescr& descr) {
  constexpr auto kBoolSize{sizeof(bool)};
  constexpr auto kCullModeSize{sizeof(render::CullMode)};
  constexpr auto kCompareOpSize{sizeof(render::CompareOp)};
  constexpr auto kPrimitiveTopologySize{sizeof(render::PrimitiveTopology)};
  constexpr auto kShaderVertexLayoutSize{sizeof(render::ShaderVertexLayout)};
  constexpr auto kUsizeSize{sizeof(usize)};
  constexpr auto kU32Size{sizeof(u32)};

  constexpr auto kShaderBindingTypeSize{sizeof(render::ShaderBindingType)};
  constexpr auto kShaderBindingScopeSize{sizeof(render::ShaderBindingScope)};
  constexpr auto kShaderMemoryLayoutSize{sizeof(render::ShaderMemoryLayout)};
  constexpr auto kShaderStageFlagsSize{sizeof(render::ShaderStageFlags)};
  constexpr auto kShaderVariableTypeSize{sizeof(render::ShaderVariableType)};
  constexpr auto kShaderImageBindingSemanticSize{
      sizeof(render::ShaderImageBindingSemantic)};

  auto total_size{
      kBoolSize +               // rasterizer.is_wireframe
      kBoolSize +               // rasterizer.is_depth_bias
      kCullModeSize +           // rasterizer.cull_mode
      kBoolSize +               // depth_stencil.is_depth_test
      kBoolSize +               // depth_stencil.is_depth_write
      kCompareOpSize +          // depth_stencil.compare_op
      kPrimitiveTopologySize +  // topology
      kShaderVertexLayoutSize   // vertex_layout
  };

  constexpr auto kShaderModuleResourceIdSize{sizeof(ShaderModuleResourceId)};

  total_size += kUsizeSize;
  total_size +=
      descr.shader_module_resource_ids.GetSize() * kShaderModuleResourceIdSize;

  total_size += kUsizeSize;

  for (const auto& define : descr.defines) {
    total_size += kUsizeSize + define.name_len;
    total_size += kUsizeSize + define.value_len;
  }

  total_size += kUsizeSize;

  for (const auto& binding : descr.bindings) {
    total_size += kUsizeSize + binding.name_len;
    total_size += kShaderBindingTypeSize;
    total_size += kShaderBindingScopeSize;
    total_size += kShaderMemoryLayoutSize;
    total_size += kU32Size;  // set
    total_size += kU32Size;  // binding
    total_size += kU32Size;  // descriptor_count
    total_size += kShaderImageBindingSemanticSize;
    total_size += kShaderStageFlagsSize;
    total_size += kUsizeSize;

    for (const auto& field : binding.fields) {
      total_size += kUsizeSize + field.name_len;
      total_size += kShaderVariableTypeSize;
      total_size += kU32Size;
    }
  }

  total_size += kUsizeSize;

  for (const auto& push_constant : descr.push_constants) {
    total_size += kUsizeSize + push_constant.name_len;
    total_size += kShaderStageFlagsSize;

    total_size += kUsizeSize;

    for (const auto& field : push_constant.fields) {
      total_size += kUsizeSize + field.name_len;
      total_size += kShaderVariableTypeSize;
      total_size += kU32Size;
    }
  }

  return total_size;
}

const schar** GetActiveShaderEngineDefines(usize& count) {
  static const schar* kActiveDefines[]{
#ifdef COMET_VALIDATION_DEBUG_PRINTF_EXT
      "COMET_VALIDATION_DEBUG_PRINTF_EXT",
#endif  // COMET_VALIDATION_DEBUG_PRINTF_EXT
#ifdef COMET_DEBUG_RENDERING
      "COMET_DEBUG_RENDERING",
#endif  // COMET_DEBUG_RENDERING
      nullptr};

  usize actual_count{0};
  while (kActiveDefines[actual_count] != nullptr) {
    ++actual_count;
  }

  count = actual_count;
  return kActiveDefines;
}

bool IsShaderEngineDefineSet(const schar* engine_define,
                             usize engine_define_len) {
  usize count{0};
  const auto** active_defines{GetActiveShaderEngineDefines(count)};

  for (usize i{0}; i < count; ++i) {
    const auto* define{active_defines[i]};

    if (AreStringsEqual(define, GetLength(define), engine_define,
                        engine_define_len)) {
      return true;
    }
  }

  return false;
}
namespace packing_internal {
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

}  // namespace packing_internal

namespace packing_internal {
void DumpShaderModules(const ShaderResourceDescr& descr,
                                              u8* buffer, usize& cursor) {
  const auto module_count{descr.shader_module_resource_ids.GetSize()};
  memory::CopyMemory(&buffer[cursor], &module_count, packing_internal::kUsizeSize);
  cursor += packing_internal::kUsizeSize;

  constexpr auto kShaderModuleResourceIdSize{sizeof(ShaderModuleResourceId)};

  for (const auto& module_resource_id : descr.shader_module_resource_ids) {
    memory::CopyMemory(&buffer[cursor], &module_resource_id,
                       kShaderModuleResourceIdSize);
    cursor += kShaderModuleResourceIdSize;
  }
}

void DumpShaderDefines(const ShaderResourceDescr& descr,
                                              u8* buffer, usize& cursor) {
  const auto define_count{descr.defines.GetSize()};
  memory::CopyMemory(&buffer[cursor], &define_count, packing_internal::kUsizeSize);
  cursor += packing_internal::kUsizeSize;

  for (const auto& define : descr.defines) {
    const auto define_name_len{define.name_len};
    memory::CopyMemory(&buffer[cursor], &define_name_len, packing_internal::kUsizeSize);
    cursor += packing_internal::kUsizeSize;

    if (define_name_len > 0) {
      memory::CopyMemory(&buffer[cursor], define.name, define_name_len);
      cursor += define_name_len;
    }

    const auto define_value_len{define.value_len};
    memory::CopyMemory(&buffer[cursor], &define_value_len,
                       packing_internal::kUsizeSize);
    cursor += packing_internal::kUsizeSize;

    if (define_value_len > 0) {
      memory::CopyMemory(&buffer[cursor], define.value, define_value_len);
      cursor += define_value_len;
    }
  }
}

void DumpBindings(const ShaderResourceDescr& descr,
                                         u8* buffer, usize& cursor) {
  const auto binding_count{descr.bindings.GetSize()};
  memory::CopyMemory(&buffer[cursor], &binding_count, packing_internal::kUsizeSize);
  cursor += packing_internal::kUsizeSize;

  for (const auto& binding : descr.bindings) {
    const auto binding_name_len{binding.name_len};
    memory::CopyMemory(&buffer[cursor], &binding_name_len,
                       packing_internal::kUsizeSize);
    cursor += packing_internal::kUsizeSize;

    if (binding_name_len > 0) {
      memory::CopyMemory(&buffer[cursor], binding.name, binding_name_len);
      cursor += binding_name_len;
    }

    memory::CopyMemory(&buffer[cursor], &binding.type,
                       packing_internal::kShaderBindingTypeSize);
    cursor += packing_internal::kShaderBindingTypeSize;

    memory::CopyMemory(&buffer[cursor], &binding.scope,
                       packing_internal::kShaderBindingScopeSize);
    cursor += packing_internal::kShaderBindingScopeSize;

    memory::CopyMemory(&buffer[cursor], &binding.layout,
                       packing_internal::kShaderMemoryLayoutSize);
    cursor += packing_internal::kShaderMemoryLayoutSize;

    memory::CopyMemory(&buffer[cursor], &binding.set, packing_internal::kU32Size);
    cursor += packing_internal::kU32Size;

    memory::CopyMemory(&buffer[cursor], &binding.binding, packing_internal::kU32Size);
    cursor += packing_internal::kU32Size;

    memory::CopyMemory(&buffer[cursor], &binding.descriptor_count,
                       packing_internal::kU32Size);
    cursor += packing_internal::kU32Size;

    memory::CopyMemory(&buffer[cursor], &binding.image_semantic,
                       packing_internal::kShaderImageBindingSemanticSize);
    cursor += packing_internal::kShaderImageBindingSemanticSize;

    memory::CopyMemory(&buffer[cursor], &binding.stages,
                       packing_internal::kShaderStageFlagsSize);
    cursor += packing_internal::kShaderStageFlagsSize;

    const auto field_count{binding.fields.GetSize()};
    memory::CopyMemory(&buffer[cursor], &field_count, packing_internal::kUsizeSize);
    cursor += packing_internal::kUsizeSize;

    for (const auto& field : binding.fields) {
      const auto field_name_len{field.name_len};
      memory::CopyMemory(&buffer[cursor], &field_name_len,
                         packing_internal::kUsizeSize);
      cursor += packing_internal::kUsizeSize;

      if (field_name_len > 0) {
        memory::CopyMemory(&buffer[cursor], field.name, field_name_len);
        cursor += field_name_len;
      }

      memory::CopyMemory(&buffer[cursor], &field.type,
                         packing_internal::kShaderVariableTypeSize);
      cursor += packing_internal::kShaderVariableTypeSize;

      memory::CopyMemory(&buffer[cursor], &field.array_count,
                         packing_internal::kU32Size);
      cursor += packing_internal::kU32Size;
    }
  }
}

void DumpPushConstants(const ShaderResourceDescr& descr,
                                              u8* buffer, usize& cursor) {
  const auto push_constant_count{descr.push_constants.GetSize()};
  memory::CopyMemory(&buffer[cursor], &push_constant_count,
                     packing_internal::kUsizeSize);
  cursor += packing_internal::kUsizeSize;

  for (const auto& push_constant : descr.push_constants) {
    const auto push_constant_name_len{push_constant.name_len};
    memory::CopyMemory(&buffer[cursor], &push_constant_name_len,
                       packing_internal::kUsizeSize);
    cursor += packing_internal::kUsizeSize;

    if (push_constant_name_len > 0) {
      memory::CopyMemory(&buffer[cursor], push_constant.name,
                         push_constant_name_len);
      cursor += push_constant_name_len;
    }

    memory::CopyMemory(&buffer[cursor], &push_constant.stages,
                       packing_internal::kShaderStageFlagsSize);
    cursor += packing_internal::kShaderStageFlagsSize;

    const auto field_count{push_constant.fields.GetSize()};
    memory::CopyMemory(&buffer[cursor], &field_count, packing_internal::kUsizeSize);
    cursor += packing_internal::kUsizeSize;

    for (const auto& field : push_constant.fields) {
      const auto field_name_len{field.name_len};
      memory::CopyMemory(&buffer[cursor], &field_name_len,
                         packing_internal::kUsizeSize);
      cursor += packing_internal::kUsizeSize;

      if (field_name_len > 0) {
        memory::CopyMemory(&buffer[cursor], field.name, field_name_len);
        cursor += field_name_len;
      }

      memory::CopyMemory(&buffer[cursor], &field.type,
                         packing_internal::kShaderVariableTypeSize);
      cursor += packing_internal::kShaderVariableTypeSize;

      memory::CopyMemory(&buffer[cursor], &field.array_count,
                         packing_internal::kU32Size);
      cursor += packing_internal::kU32Size;
    }
  }
}

}  // namespace packing_internal

Array<u8> DumpDescr(const ShaderResourceDescr& descr, memory::Allocator* allocator) {
  const auto data_size{GetSizeFromDescr(descr)};

  Array<u8> dumped_descr{allocator};
  dumped_descr.Resize(data_size);

  usize cursor{0};
  auto* buffer{dumped_descr.GetData()};

  memory::CopyMemory(&buffer[cursor], &descr.rasterizer.is_wireframe,
                     packing_internal::kBoolSize);
  cursor += packing_internal::kBoolSize;

  memory::CopyMemory(&buffer[cursor], &descr.rasterizer.is_depth_bias,
                     packing_internal::kBoolSize);
  cursor += packing_internal::kBoolSize;

  memory::CopyMemory(&buffer[cursor], &descr.rasterizer.cull_mode,
                     packing_internal::kCullModeSize);
  cursor += packing_internal::kCullModeSize;

  memory::CopyMemory(&buffer[cursor], &descr.depth_stencil.is_depth_test,
                     packing_internal::kBoolSize);
  cursor += packing_internal::kBoolSize;

  memory::CopyMemory(&buffer[cursor], &descr.depth_stencil.is_depth_write,
                     packing_internal::kBoolSize);
  cursor += packing_internal::kBoolSize;

  memory::CopyMemory(&buffer[cursor], &descr.depth_stencil.compare_op,
                     packing_internal::kCompareOpSize);
  cursor += packing_internal::kCompareOpSize;

  memory::CopyMemory(&buffer[cursor], &descr.topology,
                     packing_internal::kPrimitiveTopologySize);
  cursor += packing_internal::kPrimitiveTopologySize;

  memory::CopyMemory(&buffer[cursor], &descr.vertex_layout,
                     packing_internal::kShaderVertexLayoutSize);
  cursor += packing_internal::kShaderVertexLayoutSize;

  packing_internal::DumpShaderModules(descr, buffer, cursor);
  packing_internal::DumpShaderDefines(descr, buffer, cursor);
  packing_internal::DumpBindings(descr, buffer, cursor);
  packing_internal::DumpPushConstants(descr, buffer, cursor);

  COMET_ASSERT(cursor == dumped_descr.GetSize(),
               "DumpDescr",
               "dumped shader description size mismatch", "cursor", cursor,
               "descr_size", dumped_descr.GetSize());

  return dumped_descr;
}


ResourceFile PackShaderResource(const ShaderResource& resource,
                                CompressionMode compression_mode,
                                memory::Allocator* allocator) {
  COMET_ASSERT(allocator != nullptr, "resource::PackShaderResource", "allocator is null");
  COMET_ASSERT(resource.type_id == ShaderResource::kResourceTypeId,
               "ShaderResourceHandler::Pack",
               "shader resource type id is invalid", "resource_type_id",
               resource.type_id, "expected_type_id",
               ShaderResource::kResourceTypeId);

  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = ShaderResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{allocator};
  file.data = Array<u8>{allocator};

  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};

  Array<u8> data{allocator};
  data.Resize(kResourceIdSize + kResourceTypeIdSize);

  usize cursor{0};
  auto* buffer{data.GetData()};

  memory::CopyMemory(&buffer[cursor], &resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  COMET_ASSERT(cursor == data.GetSize(), "ShaderResourceHandler::Pack",
               "packed shader resource size mismatch", "cursor", cursor,
               "data_size", data.GetSize());

  const auto dumped_descr{DumpDescr(resource.descr, allocator)};
  file.descr_size = dumped_descr.GetSize();

  PackBytes(dumped_descr, file.compression_mode, &file.descr,
            &file.packed_descr_size);
  PackResourceData(data, file);

  return file;
}

namespace decoding_internal {
void ParseShaderModules(const u8* buffer,
                                               ShaderResourceDescr& descr,
                                               usize& cursor, memory::Allocator* allocator) {
  usize module_count{0};
  memory::CopyMemory(&module_count, &buffer[cursor], packing_internal::kUsizeSize);
  cursor += packing_internal::kUsizeSize;

  descr.shader_module_resource_ids.Reserve(module_count);

  constexpr auto kShaderModuleResourceIdSize{sizeof(ShaderModuleResourceId)};

  for (usize i{0}; i < module_count; ++i) {
    auto& module_resource_id{descr.shader_module_resource_ids.EmplaceLast()};
    memory::CopyMemory(&module_resource_id, &buffer[cursor],
                       kShaderModuleResourceIdSize);
    cursor += kShaderModuleResourceIdSize;
  }
}

void ParseShaderDefines(const u8* buffer,
                                               ShaderResourceDescr& descr,
                                               usize& cursor, memory::Allocator* allocator) {
  usize define_count{0};
  memory::CopyMemory(&define_count, &buffer[cursor], packing_internal::kUsizeSize);
  cursor += packing_internal::kUsizeSize;

  descr.defines.Reserve(define_count);

  for (usize i{0}; i < define_count; ++i) {
    auto& define_descr{descr.defines.EmplaceLast()};

    usize define_name_len{0};
    memory::CopyMemory(&define_name_len, &buffer[cursor], packing_internal::kUsizeSize);
    cursor += packing_internal::kUsizeSize;

    if (define_name_len > 0) {
      COMET_ASSERT(define_name_len < render::kMaxShaderDefineNameLen + 1,
                   "ParseShaderDefines",
                   "shader define name length is invalid", "name_len",
                   define_name_len);
      SetName(define_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
              define_name_len);
      cursor += define_name_len;
    }

    usize define_value_len{0};
    memory::CopyMemory(&define_value_len, &buffer[cursor],
                       packing_internal::kUsizeSize);
    cursor += packing_internal::kUsizeSize;

    if (define_value_len > 0) {
      COMET_ASSERT(define_value_len < render::kMaxShaderDefineValueLen + 1,
                   "ParseShaderDefines",
                   "shader define value length is invalid", "value_len",
                   define_value_len);
      SetValue(define_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
               define_value_len);
      cursor += define_value_len;
    }
  }
}

void ParseBindings(const u8* buffer,
                                          ShaderResourceDescr& descr,
                                          usize& cursor, memory::Allocator* allocator) {
  usize binding_count{0};
  memory::CopyMemory(&binding_count, &buffer[cursor], packing_internal::kUsizeSize);
  cursor += packing_internal::kUsizeSize;

  descr.bindings.Reserve(binding_count);

  for (usize i{0}; i < binding_count; ++i) {
    auto& binding_descr{descr.bindings.EmplaceLast()};

    usize binding_name_len{0};
    memory::CopyMemory(&binding_name_len, &buffer[cursor],
                       packing_internal::kUsizeSize);
    cursor += packing_internal::kUsizeSize;

    if (binding_name_len > 0) {
      SetName(binding_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
              binding_name_len);
      cursor += binding_name_len;
    }

    memory::CopyMemory(&binding_descr.type, &buffer[cursor],
                       packing_internal::kShaderBindingTypeSize);
    cursor += packing_internal::kShaderBindingTypeSize;

    memory::CopyMemory(&binding_descr.scope, &buffer[cursor],
                       packing_internal::kShaderBindingScopeSize);
    cursor += packing_internal::kShaderBindingScopeSize;

    memory::CopyMemory(&binding_descr.layout, &buffer[cursor],
                       packing_internal::kShaderMemoryLayoutSize);
    cursor += packing_internal::kShaderMemoryLayoutSize;

    memory::CopyMemory(&binding_descr.set, &buffer[cursor], packing_internal::kU32Size);
    cursor += packing_internal::kU32Size;

    memory::CopyMemory(&binding_descr.binding, &buffer[cursor],
                       packing_internal::kU32Size);
    cursor += packing_internal::kU32Size;

    memory::CopyMemory(&binding_descr.descriptor_count, &buffer[cursor],
                       packing_internal::kU32Size);
    cursor += packing_internal::kU32Size;

    memory::CopyMemory(&binding_descr.image_semantic, &buffer[cursor],
                       packing_internal::kShaderImageBindingSemanticSize);
    cursor += packing_internal::kShaderImageBindingSemanticSize;

    memory::CopyMemory(&binding_descr.stages, &buffer[cursor],
                       packing_internal::kShaderStageFlagsSize);
    cursor += packing_internal::kShaderStageFlagsSize;

    usize field_count{0};
    memory::CopyMemory(&field_count, &buffer[cursor], packing_internal::kUsizeSize);
    cursor += packing_internal::kUsizeSize;

    binding_descr.fields = Array<render::ShaderFieldDescr>{allocator};
    binding_descr.fields.Reserve(field_count);

    for (usize j{0}; j < field_count; ++j) {
      auto& field_descr{binding_descr.fields.EmplaceLast()};

      usize field_name_len{0};
      memory::CopyMemory(&field_name_len, &buffer[cursor],
                         packing_internal::kUsizeSize);
      cursor += packing_internal::kUsizeSize;

      if (field_name_len > 0) {
        COMET_ASSERT(field_name_len < render::kShaderNameMaxLen + 1,
                     "ParseBindings",
                     "shader field name length is invalid", "name_len",
                     field_name_len);
        SetName(field_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
                field_name_len);
        cursor += field_name_len;
      }

      memory::CopyMemory(&field_descr.type, &buffer[cursor],
                         packing_internal::kShaderVariableTypeSize);
      cursor += packing_internal::kShaderVariableTypeSize;

      memory::CopyMemory(&field_descr.array_count, &buffer[cursor],
                         packing_internal::kU32Size);
      cursor += packing_internal::kU32Size;
    }
  }
}

void ParsePushConstants(const u8* buffer,
                                               ShaderResourceDescr& descr,
                                               usize& cursor, memory::Allocator* allocator) {
  usize push_constant_count{0};
  memory::CopyMemory(&push_constant_count, &buffer[cursor],
                     packing_internal::kUsizeSize);
  cursor += packing_internal::kUsizeSize;

  descr.push_constants.Reserve(push_constant_count);

  for (usize i{0}; i < push_constant_count; ++i) {
    auto& push_constant_descr{descr.push_constants.EmplaceLast()};

    usize push_constant_name_len{0};
    memory::CopyMemory(&push_constant_name_len, &buffer[cursor],
                       packing_internal::kUsizeSize);
    cursor += packing_internal::kUsizeSize;

    if (push_constant_name_len > 0) {
      SetName(push_constant_descr,
              reinterpret_cast<const schar*>(&buffer[cursor]),
              push_constant_name_len);
      cursor += push_constant_name_len;
    }

    memory::CopyMemory(&push_constant_descr.stages, &buffer[cursor],
                       packing_internal::kShaderStageFlagsSize);
    cursor += packing_internal::kShaderStageFlagsSize;

    usize field_count{0};
    memory::CopyMemory(&field_count, &buffer[cursor], packing_internal::kUsizeSize);
    cursor += packing_internal::kUsizeSize;

    push_constant_descr.fields =
        Array<render::ShaderFieldDescr>{allocator};
    push_constant_descr.fields.Reserve(field_count);

    for (usize j{0}; j < field_count; ++j) {
      auto& field_descr{push_constant_descr.fields.EmplaceLast()};

      usize field_name_len{0};
      memory::CopyMemory(&field_name_len, &buffer[cursor],
                         packing_internal::kUsizeSize);
      cursor += packing_internal::kUsizeSize;

      if (field_name_len > 0) {
        COMET_ASSERT(field_name_len < render::kShaderNameMaxLen + 1,
                     "ParseBindings",
                     "shader field name length is invalid", "name_len",
                     field_name_len);
        SetName(field_descr, reinterpret_cast<const schar*>(&buffer[cursor]),
                field_name_len);
        cursor += field_name_len;
      }

      memory::CopyMemory(&field_descr.type, &buffer[cursor],
                         packing_internal::kShaderVariableTypeSize);
      cursor += packing_internal::kShaderVariableTypeSize;

      memory::CopyMemory(&field_descr.array_count, &buffer[cursor],
                         packing_internal::kU32Size);
      cursor += packing_internal::kU32Size;
    }
  }
}

}  // namespace decoding_internal

void ParseDescr(const Array<u8>& dumped_descr,
    ShaderResourceDescr& descr, memory::Allocator* allocator) {
  descr.shader_module_resource_ids =
      Array<ShaderModuleResourceId>{allocator};
  descr.defines = Array<render::ShaderDefineDescr>{allocator};
  descr.bindings = Array<render::ShaderBindingDescr>{allocator};
  descr.push_constants =
      Array<render::ShaderPushConstantDescr>{allocator};

  const auto* buffer{dumped_descr.GetData()};
  usize cursor{0};

  memory::CopyMemory(&descr.rasterizer.is_wireframe, &buffer[cursor],
                     packing_internal::kBoolSize);
  cursor += packing_internal::kBoolSize;

  memory::CopyMemory(&descr.rasterizer.is_depth_bias, &buffer[cursor],
                     packing_internal::kBoolSize);
  cursor += packing_internal::kBoolSize;

  memory::CopyMemory(&descr.rasterizer.cull_mode, &buffer[cursor],
                     packing_internal::kCullModeSize);
  cursor += packing_internal::kCullModeSize;

  memory::CopyMemory(&descr.depth_stencil.is_depth_test, &buffer[cursor],
                     packing_internal::kBoolSize);
  cursor += packing_internal::kBoolSize;

  memory::CopyMemory(&descr.depth_stencil.is_depth_write, &buffer[cursor],
                     packing_internal::kBoolSize);
  cursor += packing_internal::kBoolSize;

  memory::CopyMemory(&descr.depth_stencil.compare_op, &buffer[cursor],
                     packing_internal::kCompareOpSize);
  cursor += packing_internal::kCompareOpSize;

  memory::CopyMemory(&descr.topology, &buffer[cursor],
                     packing_internal::kPrimitiveTopologySize);
  cursor += packing_internal::kPrimitiveTopologySize;

  memory::CopyMemory(&descr.vertex_layout, &buffer[cursor],
                     packing_internal::kShaderVertexLayoutSize);
  cursor += packing_internal::kShaderVertexLayoutSize;

  decoding_internal::ParseShaderModules(buffer, descr, cursor, allocator);
  decoding_internal::ParseShaderDefines(buffer, descr, cursor, allocator);
  decoding_internal::ParseBindings(buffer, descr, cursor, allocator);
  decoding_internal::ParsePushConstants(buffer, descr, cursor, allocator);

  COMET_ASSERT(cursor == dumped_descr.GetSize(),
               "ParseDescr",
               "parsed shader description size mismatch", "cursor", cursor,
               "descr_size", dumped_descr.GetSize());
}



void UnpackShaderResource(const ResourceFile& file,
    memory::Allocator* allocator, ShaderResource* resource) {
  COMET_ASSERT(resource != nullptr, "resource::UnpackShaderResource", "resource is null");
  Array<u8> dumped_descr{allocator};
  UnpackBytes(file.compression_mode, file.descr, file.descr_size, dumped_descr);
  ParseDescr(dumped_descr, resource->descr, allocator);
  Array<u8> data{allocator};
  UnpackResourceData(file, data);
  const auto* buffer{data.GetData()};
  usize cursor{0};
  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
  COMET_ASSERT(data.GetSize() >= kResourceIdSize + kResourceTypeIdSize,
      "resource::UnpackShaderResource", "shader data is too small");
  memory::CopyMemory(&resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;
  memory::CopyMemory(&resource->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;
  COMET_ASSERT(resource->type_id == ShaderResource::kResourceTypeId,
      "resource::UnpackShaderResource", "invalid shader type");
  COMET_ASSERT(cursor == data.GetSize(), "resource::UnpackShaderResource", "shader data size mismatch");
}

}  // namespace resource
}  // namespace comet