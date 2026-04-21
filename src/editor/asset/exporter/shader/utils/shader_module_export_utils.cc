// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "shader_module_export_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/generator.h"
#include "comet/core/logger/logging.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/core/type_trait.h"
#include "comet/rendering/label/rendering_shader_label.h"
#include "comet/rendering/type/rendering_shader_type.h"
#include "comet/resource/shader/shader_resource.h"

namespace comet {
namespace editor {
namespace asset {
bool PopulateSpvShaderCode(CTStringView asset_abs_path, schar* code,
                           memory::Allocator* allocator,
                           resource::ShaderModuleResource& shader_module) {
  COMET_ASSERT(code != nullptr,
               "shader_module_export_utils::PopulateSpvShaderCode",
               "shader code is null");
  COMET_ASSERT(allocator != nullptr,
               "shader_module_export_utils::PopulateSpvShaderCode",
               "allocator is null");

  shaderc::Compiler compiler;

  shaderc::CompileOptions options;
#ifdef COMET_DEBUG_SHADER
  options.SetOptimizationLevel(shaderc_optimization_level_zero);
  options.SetGenerateDebugInfo();
#else
  options.SetOptimizationLevel(shaderc_optimization_level_size);
#endif  // COMET_DEBUG_SHADER

  AddSpvMacroDefinitions(options);
  shaderc_shader_kind shader_kind{};

  switch (shader_module.descr.stage) {
    case rendering::ShaderStage::Compute:
      shader_kind = shaderc_shader_kind::shaderc_glsl_compute_shader;
      break;
    case rendering::ShaderStage::Vertex:
      shader_kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
      break;
    case rendering::ShaderStage::Fragment:
      shader_kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
      break;
    default:
      COMET_LOG_ERROR(LoggerType::External,
                      "shader_module_export_utils::PopulateSpvShaderCode",
                      "shader stage is unsupported", "stage",
                      rendering::GetShaderStageLabel(shader_module.descr.stage),
                      "stage_value", ToUnderlying(shader_module.descr.stage),
                      "asset_path", asset_abs_path);
      break;
  }

  const auto tmp_input_file_name{GetName(asset_abs_path)};
#ifdef COMET_WIDE_TCHAR
  const auto* input_file_name{GenerateForOneFrame<schar>(
      tmp_input_file_name.GetCTStr(), tmp_input_file_name.GetLength())};
#else
  const auto* input_file_name{tmp_input_file_name.GetCTStr()};
#endif  // COMET_WIDE_TCHAR

  const auto result{
      compiler.CompileGlslToSpv(code, shader_kind, input_file_name, options)};

  if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
    COMET_LOG_ERROR(LoggerType::External,
                    "shader_module_export_utils::PopulateSpvShaderCode",
                    "shader compilation failed", "asset_path", asset_abs_path);
    COMET_LOG_ERROR(LoggerType::External,
                    "shader_module_export_utils::PopulateSpvShaderCode",
                    "shader compiler error", "asset_path", asset_abs_path,
                    "error", result.GetErrorMessage());
    return false;
  }

  shader_module.data = Array<u8>{allocator};
  shader_module.data.Resize((result.cend() - result.cbegin()) * sizeof(u32));
  memory::CopyMemory(shader_module.data.GetData(), result.cbegin(),
                     shader_module.data.GetSize());
  return true;
}

bool PopulateGlShaderCode(schar* code, usize code_len,
                          memory::Allocator* allocator,
                          resource::ShaderModuleResource& shader_module) {
  COMET_ASSERT(code != nullptr,
               "shader_module_export_utils::PopulateGlShaderCode",
               "shader code is null");
  COMET_ASSERT(allocator != nullptr,
               "shader_module_export_utils::PopulateGlShaderCode",
               "allocator is null");

  constexpr schar kVersionToken[]{"#version"};
  constexpr usize kVersionTokenLen{GetLength(kVersionToken)};
  constexpr schar kNewLine{'\n'};

  usize version_line_end{0};
  usize version_line_len{0};

  // Find first line and check for #version.
  for (usize i{0}; i < code_len; ++i) {
    if (code[i] == kNewLine) {
      version_line_end = i;
      break;
    }
  }

  if (version_line_end > 0 &&
      Compare(code, kVersionToken, kVersionTokenLen) == 0) {
    version_line_len = version_line_end + 1;  // Include newline.
  }

  const auto local_size_value_char_count{
      GetCharCount(rendering::kShaderLocalSize)};

  auto* local_size_value{
      GenerateForOneFrame<schar>(local_size_value_char_count)};
  COMET_ASSERT(local_size_value != nullptr,
               "shader_module_export_utils::PopulateGlShaderCode",
               "local size buffer allocation failed");

  usize local_size_value_len;
  ConvertToStr(rendering::kShaderLocalSize, local_size_value,
               local_size_value_char_count, &local_size_value_len);

  constexpr schar kDefinePrefix[]{"#define "};
  constexpr usize kDefinePrefixLen{GetLength(kDefinePrefix)};

  constexpr schar kLocalSizeDefine[]{"LOCAL_SIZE "};
  constexpr usize kLocalSizeDefineLen{GetLength(kLocalSizeDefine)};

  usize engine_define_count;
  const auto** engine_defines{
      resource::GetActiveShaderEngineDefines(engine_define_count)};

  shader_module.data = Array<u8>{allocator};
  // For now, only null terminators.
  constexpr usize kAdditionalSizeEstimate{1};

  const auto estimated_size =
      code_len + kAdditionalSizeEstimate + kDefinePrefixLen +
      kLocalSizeDefineLen + local_size_value_len + 1 +
      engine_define_count *
          (rendering::kMaxShaderDefineNameLen + kDefinePrefixLen + 1);

  shader_module.data.Resize(estimated_size);

  usize cursor{0};

  if (version_line_len > 0) {
    memory::CopyMemory(shader_module.data.GetData() + cursor, code,
                       version_line_len);
    cursor += version_line_len;
  }

  memory::CopyMemory(shader_module.data.GetData() + cursor, kDefinePrefix,
                     kDefinePrefixLen);
  cursor += kDefinePrefixLen;

  memory::CopyMemory(shader_module.data.GetData() + cursor, kLocalSizeDefine,
                     kLocalSizeDefineLen);
  cursor += kLocalSizeDefineLen;

  memory::CopyMemory(shader_module.data.GetData() + cursor, local_size_value,
                     local_size_value_len);
  cursor += local_size_value_len;

  memory::CopyMemory(shader_module.data.GetData() + cursor, &kNewLine, 1);
  ++cursor;

  for (usize i{0}; i < engine_define_count; ++i) {
    memory::CopyMemory(shader_module.data.GetData() + cursor, kDefinePrefix,
                       kDefinePrefixLen);
    cursor += kDefinePrefixLen;

    const auto engine_define_len{GetLength(engine_defines[i])};

    memory::CopyMemory(shader_module.data.GetData() + cursor, engine_defines[i],
                       engine_define_len);
    cursor += engine_define_len;

    memory::CopyMemory(shader_module.data.GetData() + cursor, &kNewLine, 1);
    ++cursor;
  }

  const auto code_offset{version_line_len};
  const auto effective_code_len{code_len - code_offset};

  memory::CopyMemory(shader_module.data.GetData() + cursor, code + code_offset,
                     effective_code_len);
  cursor += effective_code_len;

  // Just in case.
  constexpr schar kNullTerminator{'\0'};
  memory::CopyMemory(shader_module.data.GetData() + cursor, &kNullTerminator,
                     1);
  ++cursor;

  shader_module.data.Resize(cursor);
  return true;
}

void AddSpvMacroDefinitions(shaderc::CompileOptions& options) {
  // Inject local workgroup.
  const auto local_size_value_char_count{
      GetCharCount(rendering::kShaderLocalSize)};

  auto* local_size{GenerateForOneFrame<schar>(local_size_value_char_count)};
  COMET_ASSERT(local_size != nullptr,
               "shader_module_export_utils::AddSpvMacroDefinitions",
               "local size buffer allocation failed");

  usize local_size_len;

  ConvertToStr(rendering::kShaderLocalSize, local_size,
               local_size_value_char_count, &local_size_len);
  options.AddMacroDefinition("LOCAL_SIZE", GetLength("LOCAL_SIZE"), local_size,
                             local_size_len);

  // Inject active engine defines for filtering shader code.
  usize count;
  const auto** active_defines{resource::GetActiveShaderEngineDefines(count)};

  for (usize i{0}; i < count; ++i) {
    options.AddMacroDefinition(active_defines[i]);
  }
}
}  // namespace asset
}  // namespace editor
}  // namespace comet