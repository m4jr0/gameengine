// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "shader_resource.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"

namespace comet {
namespace resource {
const ResourceTypeId ShaderResource::kResourceTypeId{COMET_STRING_ID("shader")};

usize GetSizeFromDescr(const ShaderResourceDescr& descr) {
  constexpr auto kBoolSize{sizeof(bool)};
  constexpr auto kCullModeSize{sizeof(rendering::CullMode)};
  constexpr auto kCompareOpSize{sizeof(rendering::CompareOp)};
  constexpr auto kPrimitiveTopologySize{sizeof(rendering::PrimitiveTopology)};
  constexpr auto kShaderVertexLayoutSize{sizeof(rendering::ShaderVertexLayout)};
  constexpr auto kUsizeSize{sizeof(usize)};
  constexpr auto kU32Size{sizeof(u32)};

  constexpr auto kShaderBindingTypeSize{sizeof(rendering::ShaderBindingType)};
  constexpr auto kShaderBindingScopeSize{sizeof(rendering::ShaderBindingScope)};
  constexpr auto kShaderMemoryLayoutSize{sizeof(rendering::ShaderMemoryLayout)};
  constexpr auto kShaderStageFlagsSize{sizeof(rendering::ShaderStageFlags)};
  constexpr auto kShaderVariableTypeSize{sizeof(rendering::ShaderVariableType)};
  constexpr auto kShaderImageBindingSemanticSize{
      sizeof(rendering::ShaderImageBindingSemantic)};

  auto total_size{kBoolSize + kBoolSize + kCullModeSize + kBoolSize +
                  kBoolSize + kCompareOpSize + kPrimitiveTopologySize +
                  kShaderVertexLayoutSize};

  total_size += kUsizeSize;

  for (const auto& module_path : descr.shader_module_paths) {
    total_size += kUsizeSize;
    total_size += module_path.GetLengthWithNullTerminator() * sizeof(tchar);
  }

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
    total_size += kU32Size;
    total_size += kU32Size;
    total_size += kU32Size;
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
#ifdef COMET_DEBUG_CULLING
      "COMET_DEBUG_CULLING",
#endif  // COMET_DEBUG_CULLING
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
  usize count;
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

ResourceId GetDefaultShaderResourceId() {
  return resource::GenerateResourceIdFromPath<resource::ShaderResource>(
      COMET_TCHAR("shaders/vulkan/default_shader.vk.cshader"));
}
}  // namespace resource
}  // namespace comet