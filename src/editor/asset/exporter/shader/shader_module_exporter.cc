// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "shader_module_exporter.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/generator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace editor {
namespace asset {
bool ShaderModuleExporter::IsCompatible(CTStringView extension) const {
  return extension == COMET_TCHAR("comp") || extension == COMET_TCHAR("vert") ||
         extension == COMET_TCHAR("frag");
}

void ShaderModuleExporter::PopulateFiles(ResourceFilesContext& context) const {
  auto& asset_descr{context.asset_descr};
  const auto driver_keyword_pos{
      asset_descr.asset_path.GetNthToLastIndexOf(COMET_TCHAR('.'), 2)};

  if (driver_keyword_pos == kInvalidIndex) {
    COMET_LOG_GLOBAL_ERROR(
        "Unable to retrieve driver keyword from asset shader path: ",
        asset_descr.asset_path, ".");
    return;
  }

  auto driver_keyword{
      asset_descr.asset_path.GenerateSubString(driver_keyword_pos + 1)};
  ReplaceExtension(COMET_TCHAR(""), driver_keyword);
  auto shader_keyword{GetExtension(asset_descr.asset_path)};

  rendering::ShaderModuleType shader_type{rendering::ShaderModuleType::Unknown};
  rendering::DriverType driver_type{rendering::DriverType::Unknown};

  ShaderCodeContext shader_code_context{};
  shader_code_context.code =
      GenerateForOneFrame<schar>(ShaderCodeContext::kMaxShaderCodeLen_);
  shader_code_context.asset_abs_path = asset_descr.asset_abs_path.GetCTStr();
  shader_code_context.allocator = context.allocator;

  if (driver_keyword == COMET_TCHAR("gl")) {
    driver_type = rendering::DriverType::OpenGl;
  } else if (driver_keyword == COMET_TCHAR("vk")) {
    driver_type = rendering::DriverType::Vulkan;
  }

  if (shader_keyword == COMET_TCHAR("comp")) {
    shader_type = rendering::ShaderModuleType::Compute;
  } else if (shader_keyword == COMET_TCHAR("vert")) {
    shader_type = rendering::ShaderModuleType::Vertex;
  } else if (shader_keyword == COMET_TCHAR("frag")) {
    shader_type = rendering::ShaderModuleType::Fragment;
  }

  if (shader_type == rendering::ShaderModuleType::Unknown) {
    COMET_LOG_GLOBAL_ERROR(
        "Unknown or unsupported shader type! Keyword retrieved is ",
        shader_keyword, ".");
  } else if (driver_type == rendering::DriverType::Unknown) {
    COMET_LOG_GLOBAL_ERROR(
        "Unknown or unsupported driver type! Keyword retrieved is ",
        driver_keyword, ".");
  }

  resource::ShaderModuleResource shader_module{};
  shader_module.id =
      resource::GenerateResourceIdFromPath<resource::ShaderModuleResource>(
          asset_descr.asset_path);
  shader_module.type_id = resource::ShaderModuleResource::kResourceTypeId;
  shader_module.descr.shader_type = shader_type;
  shader_module.descr.driver_type = driver_type;

  {
    job::CounterGuard guard{};

    job::Scheduler::Get().KickAndWait(job::GenerateIOJobDescr(
        OnShaderModuleLoading, &shader_code_context, guard.GetCounter()));
  }

  COMET_LOG_GLOBAL_DEBUG("Processing shader module at: ",
                         shader_code_context.asset_abs_path, "...");

  switch (driver_type) {
    case rendering::DriverType::Vulkan:
      GenerateSpvShaderCode(shader_code_context, shader_module);
      break;
    default:
    case rendering::DriverType::OpenGl:
      GenerateStringShaderCode(shader_code_context, shader_module);
      break;
  }

  COMET_LOG_GLOBAL_DEBUG("Shader module processed at: ",
                         shader_code_context.asset_abs_path);
  context.files.PushBack(
      resource::ResourceManager::Get().GetShaderModules()->Pack(
          shader_module, compression_mode_));
}

void ShaderModuleExporter::GenerateSpvShaderCode(
    const ShaderCodeContext& shader_code_context,
    resource::ShaderModuleResource& shader_module) {
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

  switch (shader_module.descr.shader_type) {
    case rendering::ShaderModuleType::Compute:
      shader_kind = shaderc_shader_kind::shaderc_glsl_compute_shader;
      break;
    case rendering::ShaderModuleType::Vertex:
      shader_kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
      break;
    case rendering::ShaderModuleType::Fragment:
      shader_kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
      break;
    default:
      COMET_LOG_GLOBAL_ERROR(
          "Unknown or unsupported shader module type provided: ",
          static_cast<std::underlying_type_t<rendering::ShaderModuleType>>(
              shader_module.descr.shader_type),
          "!");
      break;
  }

  const auto tmp_input_file_name{GetName(shader_code_context.asset_abs_path)};
#ifdef COMET_WIDE_TCHAR
  const auto* input_file_name{GenerateForOneFrame<schar>(
      tmp_input_file_name.GetCTStr(), tmp_input_file_name.GetLength())};
#else
  const auto* input_file_name{tmp_input_file_name.GetCTStr()};
#endif  // COMET_WIDE_TCHAR

  const auto result{compiler.CompileGlslToSpv(
      shader_code_context.code, shader_kind, input_file_name, options)};

  if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
    COMET_LOG_GLOBAL_ERROR("Shaderc compilation error! At ",
                           shader_code_context.asset_abs_path, ".");
    COMET_LOG_GLOBAL_ERROR("\t", result.GetErrorMessage());
  }

  shader_module.data = Array<u8>{shader_code_context.allocator};
  shader_module.data.Resize((result.cend() - result.cbegin()) * sizeof(u32));
  memory::CopyMemory(shader_module.data.GetData(), result.cbegin(),
                     shader_module.data.GetSize());
}

void ShaderModuleExporter::GenerateStringShaderCode(
    const ShaderCodeContext& shader_code_context,
    resource::ShaderModuleResource& shader_module) {
  constexpr schar kVersionToken[]{"#version"};
  constexpr usize kVersionTokenLen{GetLength(kVersionToken)};
  constexpr schar kNewLine{'\n'};

  usize version_line_end{0};
  usize version_line_len{0};

  // Find first line and check for #version.
  for (usize i{0}; i < shader_code_context.code_len; ++i) {
    if (shader_code_context.code[i] == kNewLine) {
      version_line_end = i;
      break;
    }
  }

  if (version_line_end > 0 &&
      Compare(shader_code_context.code, kVersionToken, kVersionTokenLen) == 0) {
    version_line_len = version_line_end + 1;  // Include newline.
  }

  auto local_size_value_char_count{GetCharCount(rendering::kShaderLocalSize)};
  auto* local_size_value{
      GenerateForOneFrame<schar>(local_size_value_char_count)};
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

  shader_module.data = Array<u8>{shader_code_context.allocator};
  // For now, only null terminators.
  constexpr usize kAdditionalSizeEstimate{1};

  // Add a few (kAdditionalSizeEstimate) extra characters when injecting macros.
  // Also, add define lengths (including LOCAL_SIZE) and their new lines.
  auto upper_bound_size_estimate{
      shader_code_context.code_len +
      // Additional space.
      kAdditionalSizeEstimate +
      // LOCAL_SIZE.
      kDefinePrefixLen + kLocalSizeDefineLen + local_size_value_len + 1 +
      // Engine defines.
      engine_define_count *
          (rendering::kMaxShaderDefineNameLen + kDefinePrefixLen + 1)};

  shader_module.data.Resize(shader_code_context.code_len +
                            upper_bound_size_estimate);

  usize cursor{0};

  if (version_line_len > 0) {
    memory::CopyMemory(shader_module.data.GetData() + cursor,
                       shader_code_context.code, version_line_len);
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

    auto engine_define_len{GetLength(engine_defines[i])};

    memory::CopyMemory(shader_module.data.GetData() + cursor, engine_defines[i],
                       engine_define_len);
    cursor += engine_define_len;

    memory::CopyMemory(shader_module.data.GetData() + cursor, &kNewLine, 1);
    ++cursor;
  }

  auto code_len{shader_code_context.code_len - version_line_end};
  memory::CopyMemory(shader_module.data.GetData() + cursor,
                     shader_code_context.code + version_line_end, code_len);
  cursor += code_len;

  // Just in case.
  constexpr schar kNullTerminator{'\0'};
  memory::CopyMemory(shader_module.data.GetData() + cursor, &kNullTerminator,
                     1);
  ++cursor;
}

void ShaderModuleExporter::AddSpvMacroDefinitions(
    shaderc::CompileOptions& options) {
  // Inject local workgroup.
  auto local_size_value_char_count{GetCharCount(rendering::kShaderLocalSize)};
  auto* local_size{GenerateForOneFrame<schar>(local_size_value_char_count)};
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

void ShaderModuleExporter::OnShaderModuleLoading(
    job::IOJobParamsHandle params_handle) {
  auto* shader_code_context{static_cast<ShaderCodeContext*>(params_handle)};
  ReadStrFromFile(
      shader_code_context->asset_abs_path, shader_code_context->code,
      ShaderCodeContext::kMaxShaderCodeLen_, &shader_code_context->code_len);
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
