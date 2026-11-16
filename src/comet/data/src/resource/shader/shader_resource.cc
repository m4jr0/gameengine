// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "shader_resource.h"
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

usize GetTStringByteSize(const TString& str) {
  return str.GetLengthWithNullTerminator() * sizeof(tchar);
}
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

}  // namespace resource
}  // namespace comet