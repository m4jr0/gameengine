// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "shader_module_exporter.h"

#include "shaderc/shaderc.hpp"

#include "comet/core/file_system.h"
#include "comet/core/string.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/shader_module_resource.h"

namespace comet {
namespace editor {
namespace asset {
bool ShaderModuleExporter::IsCompatible(std::string_view extension) const {
  return extension == "vert" || extension == "frag";
}

std::vector<resource::ResourceFile> ShaderModuleExporter::GetResourceFiles(
    AssetDescr& asset_descr) const {
  const auto driver_keyword_pos{
      GetLastNthPos(asset_descr.asset_path, std::string("."), 2)};

  std::string driver_keyword;

  if (driver_keyword_pos != kInvalidIndex) {
    driver_keyword = ReplaceExtensionToCopy(
        "", asset_descr.asset_path.substr(driver_keyword_pos + 1,
                                          asset_descr.asset_path.size()));
  } else {
    COMET_LOG_GLOBAL_ERROR(
        "Unable to retrieve driver keyword from asset shader path: ",
        asset_descr.asset_path, ".");
  }

  auto shader_keyword{GetExtension(asset_descr.asset_path)};

  rendering::ShaderModuleType shader_type{rendering::ShaderModuleType::Unknown};
  rendering::DriverType driver_type{rendering::DriverType::Unknown};

  if (driver_keyword == "gl") {
    driver_type = rendering::DriverType::OpenGl;
  } else if (driver_keyword == "vk") {
    driver_type = rendering::DriverType::Vulkan;
  }

  if (shader_keyword == "vert") {
    shader_type = rendering::ShaderModuleType::Vertex;
  } else if (shader_keyword == "frag") {
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

  std::string shader_code;
  ReadStrFromFile(asset_descr.asset_abs_path, shader_code);

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

      const auto result{compiler.CompileGlslToSpv(
          shader_code.c_str(), shader_kind,
          GetNameView(asset_descr.asset_abs_path).data(), options)};

      if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        COMET_LOG_GLOBAL_ERROR("Shaderc compilation error! At ",
                               asset_descr.asset_abs_path, ".");
        COMET_LOG_GLOBAL_ERROR("\t", result.GetErrorMessage());
      }

      shader.data.resize((result.cend() - result.cbegin()) * sizeof(u32));
      std::memcpy(shader.data.data(), result.cbegin(), shader.data.size());
      break;
    }
    default:
    case rendering::DriverType::OpenGl: {
      shader.data.resize(shader_code.size());
      std::memcpy(shader.data.data(), shader_code.c_str(), shader.data.size());
      break;
    }
  }

  return std::vector<resource::ResourceFile>{
      resource::ResourceManager::Get().GetResourceFile(shader,
                                                       compression_mode_)};
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
