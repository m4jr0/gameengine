// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "shader_module_exporter.h"

#include "shaderc/shaderc.hpp"

#include "comet/core/c_string.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/generator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/shader_module_resource.h"

namespace comet {
namespace editor {
namespace asset {
bool ShaderModuleExporter::IsCompatible(CTStringView extension) const {
  return extension == COMET_TCHAR("vert") || extension == COMET_TCHAR("frag");
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

  if (driver_keyword == COMET_TCHAR("gl")) {
    driver_type = rendering::DriverType::OpenGl;
  } else if (driver_keyword == COMET_TCHAR("vk")) {
    driver_type = rendering::DriverType::Vulkan;
  }

  if (shader_keyword == COMET_TCHAR("vert")) {
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

  resource::ShaderModuleResource shader{};
  shader.id = resource::GenerateResourceIdFromPath(asset_descr.asset_path);
  shader.type_id = resource::ShaderModuleResource::kResourceTypeId;
  shader.descr.shader_type = shader_type;
  shader.descr.driver_type = driver_type;

  ShaderCodeContext shader_code_context{};
  shader_code_context.asset_abs_path = asset_descr.asset_abs_path.GetCTStr();

  job::Scheduler::Get().KickAndWait(
      job::GenerateIOJobDescr(OnShaderModuleLoading, &shader_code_context));

  COMET_LOG_GLOBAL_DEBUG("Processing shader module at: ",
                         shader_code_context.asset_abs_path, "...");

  switch (driver_type) {
    case rendering::DriverType::Vulkan: {
      shaderc::Compiler compiler;

      shaderc::CompileOptions options;
#ifdef COMET_DEBUG
      options.SetGenerateDebugInfo();
#else
      options.SetOptimizationLevel(shaderc_optimization_level_size);
#endif  // COMET_DEBUG

      shaderc_shader_kind shader_kind{};

      switch (shader_type) {
        case rendering::ShaderModuleType::Vertex:
          shader_kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
          break;
        default:
        case rendering::ShaderModuleType::Fragment:
          shader_kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
          break;
      }

      const auto tmp_input_file_name{GetName(asset_descr.asset_abs_path)};
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
                               asset_descr.asset_abs_path, ".");
        COMET_LOG_GLOBAL_ERROR("\t", result.GetErrorMessage());
      }

      shader.data.resize((result.cend() - result.cbegin()) * sizeof(u32));
      memory::CopyMemory(shader.data.data(), result.cbegin(),
                         shader.data.size());
      break;
    }
    default:
    case rendering::DriverType::OpenGl: {
      shader.data.resize(shader_code_context.code_len);
      memory::CopyMemory(shader.data.data(), shader_code_context.code,
                         shader.data.size());
      break;
    }
  }

  COMET_LOG_GLOBAL_DEBUG("Shader module processed at: ",
                         shader_code_context.asset_abs_path);
  context.files.PushBack(resource::ResourceManager::Get().GetResourceFile(
      shader, compression_mode_));
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
