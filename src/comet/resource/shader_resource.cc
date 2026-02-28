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
  constexpr auto kPrimitiveTopologySize{sizeof(rendering::PrimitiveTopology)};
  constexpr auto kUsizeSize(sizeof(usize));
  constexpr auto kShaderVertexAttributeTypeSize(
      sizeof(rendering::ShaderVertexAttributeType));
  constexpr auto kShaderUniformTypeSize(sizeof(rendering::ShaderVariableType));
  constexpr auto kShaderUniformScopeSize(sizeof(rendering::ShaderUniformScope));
  constexpr auto kShaderConstantTypeSize(sizeof(rendering::ShaderVariableType));
  constexpr auto kShaderStoragePropertyTypeSize(
      sizeof(rendering::ShaderVariableType));
  constexpr auto kShaderStageFlagsSize(sizeof(rendering::ShaderStageFlags));

  // is_wireframe size, cull_mode size, topology size, and shader_module_paths
  // size.
  auto total_size{kBoolSize + kCullModeSize + kPrimitiveTopologySize +
                  kUsizeSize};

  for (const auto& module_path : descr.shader_module_paths) {
    total_size += kUsizeSize +
                  (module_path.GetLengthWithNullTerminator()) * sizeof(tchar);
  }

  total_size += kUsizeSize;

  for (const auto& define : descr.defines) {
    total_size += kUsizeSize + define.name_len;
    total_size += kUsizeSize + define.value_len;
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

    total_size += kUsizeSize + buffer.engine_define_len;
  }

  return total_size;
}

const schar** GetActiveShaderEngineDefines(usize& count) {
  static const schar* kActiveDefines[]{
#if defined(COMET_VALIDATION_DEBUG_PRINTF_EXT)
      "COMET_VALIDATION_DEBUG_PRINTF_EXT",
#endif
#if defined(COMET_DEBUG_RENDERING)
      "COMET_DEBUG_RENDERING",
#endif
#if defined(COMET_DEBUG_CULLING)
      "COMET_DEBUG_CULLING",
#endif
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
}  // namespace resource
}  // namespace comet
