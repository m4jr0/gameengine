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
}  // namespace resource
}  // namespace comet