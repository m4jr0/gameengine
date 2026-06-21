// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "assetc_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "shader_module_exporter.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/data.h"
#include "comet/render.h"
#include "comet/runtime.h"
#include "exporter/shader/utils/shader_module_export_utils.h"

namespace comet {
namespace tool {
namespace assetc {
bool ShaderModuleExporter::IsCompatible(CTStringView extension) const {
  return extension == COMET_TCHAR("comp") || extension == COMET_TCHAR("vert") ||
         extension == COMET_TCHAR("frag");
}

void ShaderModuleExporter::PopulateFiles(ResourceFilesContext& context) const {
  auto& asset_descr{context.asset_descr};
  const auto driver_keyword_pos{
      asset_descr.asset_path.GetNthToLastIndexOf(COMET_TCHAR('.'), 2)};

  if (driver_keyword_pos == kInvalidIndex) {
    COMET_LOG_ERROR(LoggerType::External, "ShaderModuleExporter::PopulateFiles",
                    "driver keyword could not be resolved", "asset_path",
                    asset_descr.asset_path);
    return;
  }

  auto driver_keyword{
      asset_descr.asset_path.GenerateSubString(driver_keyword_pos + 1)};
  ReplaceExtension(COMET_TCHAR(""), driver_keyword);
  const auto shader_keyword{GetExtension(asset_descr.asset_path)};

  render::ShaderStage stage{render::ShaderStage::Unknown};
  render::DriverType driver_type{render::DriverType::Unknown};

  ShaderCodeContext shader_code_context{};
  shader_code_context.code =
      GenerateFrameString<schar>(ShaderCodeContext::kMaxShaderCodeLen_);
  shader_code_context.asset_abs_path = asset_descr.asset_abs_path.GetCTStr();
  shader_code_context.allocator = context.allocator;

  if (driver_keyword == COMET_TCHAR("gl")) {
    driver_type = render::DriverType::OpenGl;
  } else if (driver_keyword == COMET_TCHAR("vk")) {
    driver_type = render::DriverType::Vulkan;
  }

  if (shader_keyword == COMET_TCHAR("comp")) {
    stage = render::ShaderStage::Compute;
  } else if (shader_keyword == COMET_TCHAR("vert")) {
    stage = render::ShaderStage::Vertex;
  } else if (shader_keyword == COMET_TCHAR("frag")) {
    stage = render::ShaderStage::Fragment;
  }

  if (stage == render::ShaderStage::Unknown) {
    COMET_LOG_ERROR(LoggerType::External, "ShaderModuleExporter::PopulateFiles",
                    "shader stage is unsupported", "shader_keyword",
                    shader_keyword, "asset_path", asset_descr.asset_path);
    return;
  } else if (driver_type == render::DriverType::Unknown) {
    COMET_LOG_ERROR(LoggerType::External, "ShaderModuleExporter::PopulateFiles",
                    "driver type is unsupported", "driver_keyword",
                    driver_keyword, "asset_path", asset_descr.asset_path);
    return;
  }

  resource::ShaderModuleResource shader_module{};
  const auto shader_module_resource_id{
      resource::GenerateResourceIdFromPath<resource::ShaderModuleResource>(
          asset_descr.asset_path)};
  shader_module.id = shader_module_resource_id.GetValue();
  shader_module.type_id = resource::ShaderModuleResource::kResourceTypeId;
  shader_module.descr.stage = stage;
  shader_module.descr.driver_type = driver_type;

  {
    job::CounterGuard guard{};

    job::Scheduler::Get().KickAndWait(job::GenerateIOJobDescr(
        OnShaderModuleLoading, &shader_code_context, guard.GetCounter()));
  }

  COMET_LOG_DEBUG(LoggerType::External, "ShaderModuleExporter::PopulateFiles",
                  "processing shader module", "asset_path",
                  shader_code_context.asset_abs_path);

  auto is_success{false};

  switch (driver_type) {
    case render::DriverType::Vulkan:
      is_success = PopulateSpvShaderCode(
          shader_code_context.asset_abs_path, shader_code_context.code,
          shader_code_context.allocator, shader_module);
      break;

    case render::DriverType::OpenGl:
    default:
      is_success = PopulateGlShaderCode(
          shader_code_context.code, shader_code_context.code_len,
          shader_code_context.allocator, shader_module);
      break;
  }

  if (!is_success) {
    COMET_LOG_ERROR(LoggerType::External, "ShaderModuleExporter::PopulateFiles",
                    "shader module processing failed", "asset_path",
                    shader_code_context.asset_abs_path, "driver_type",
                    render::GetDriverTypeLabel(driver_type),
                    "driver_type_value", ToUnderlying(driver_type), "stage",
                    render::GetShaderStageLabel(stage), "stage_value",
                    ToUnderlying(stage));

    return;
  }

  COMET_LOG_DEBUG(LoggerType::External, "ShaderModuleExporter::PopulateFiles",
                  "shader module processed", "asset_path",
                  shader_code_context.asset_abs_path);
  context.files.PushLast(
      resource::ResourceManager::Get().GetShaderModules()->Pack(
          shader_module, compression_mode_));
}

void ShaderModuleExporter::OnShaderModuleLoading(
    job::IOJobParamsHandle params_handle) {
  auto* shader_code_context{static_cast<ShaderCodeContext*>(params_handle)};
  COMET_ASSERT(shader_code_context != nullptr,
               "ShaderModuleExporter::OnShaderModuleLoading",
               "shader code context is null");
  COMET_ASSERT(shader_code_context->code != nullptr,
               "ShaderModuleExporter::OnShaderModuleLoading",
               "shader code buffer is null");

  ReadStrFromFile(
      shader_code_context->asset_abs_path, shader_code_context->code,
      ShaderCodeContext::kMaxShaderCodeLen_, &shader_code_context->code_len);
}
}  // namespace assetc
}  // namespace tool
}  // namespace comet
