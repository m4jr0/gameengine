// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "shader_exporter.h"

#include "shaderc/shaderc.hpp"

#include "comet/core/engine.h"
#include "comet/resource/shader_resource.h"
#include "comet/utils/file_system.h"
#include "comet/utils/string.h"

namespace comet {
namespace editor {
namespace asset {
bool ShaderExporter::IsCompatible(const std::string& extension) {
  return extension == "vert" || extension == "frag";
}

std::vector<resource::ResourceFile> ShaderExporter::GetResourceFiles(
    AssetDescr& asset_descr) {
  const auto driver_keyword_pos{utils::string::GetSubStrNthPos(
      asset_descr.asset_path, std::string("."), 2)};

  std::string driver_keyword;

  if (driver_keyword_pos != kInvalidIndex) {
    driver_keyword = utils::filesystem::ReplaceExtension(
        asset_descr.asset_path.substr(driver_keyword_pos + 1,
                                      asset_descr.asset_path.size()),
        "");
  } else {
    COMET_LOG_GLOBAL_ERROR(
        "Unable to retrieve driver keyword from asset shader path: ",
        asset_descr.asset_path, ".");
  }

  std::string shader_keyword{
      utils::filesystem::GetExtension(asset_descr.asset_path)};

  resource::ShaderType shader_type{resource::ShaderType::Unknown};
  rendering::DriverType driver_type{rendering::DriverType::Unknown};

  if (driver_keyword == "gl") {
    driver_type = rendering::DriverType::OpenGl;
  } else if (driver_keyword == "vk") {
    driver_type = rendering::DriverType::Vulkan;
  }

  if (shader_keyword == "vert") {
    shader_type = resource::ShaderType::Vertex;
  } else if (shader_keyword == "frag") {
    shader_type = resource::ShaderType::Fragment;
  }

  if (shader_type == resource::ShaderType::Unknown) {
    COMET_LOG_GLOBAL_ERROR(
        "Unknown or unsupported shader type! Keyword retrieved is ",
        shader_keyword, ".");
  } else if (driver_type == rendering::DriverType::Unknown) {
    COMET_LOG_GLOBAL_ERROR(
        "Unknown or unsupported driver type! Keyword retrieved is ",
        driver_keyword, ".");
  }

  resource::ShaderResource shader{};
  shader.id = resource::GenerateResourceIdFromPath(asset_descr.asset_path);
  shader.type_id = resource::ShaderResource::kResourceTypeId;
  shader.descr.shader_type = shader_type;
  shader.descr.driver_type = driver_type;

  std::string shader_code;
  utils::filesystem::ReadStrFromFile(asset_descr.asset_abs_path, shader_code);

  switch (driver_type) {
    case rendering::DriverType::Vulkan: {
      shaderc::Compiler compiler;

      shaderc::CompileOptions options;
      // As we use reflection, we need to keep binding names.
      // options.SetOptimizationLevel(shaderc_optimization_level_size);

      shaderc_shader_kind shader_kind{};

      switch (shader_type) {
        case resource::ShaderType::Vertex:
          shader_kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
          break;
        default:
        case resource::ShaderType::Fragment:
          shader_kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
          break;
      }

      const auto result{compiler.CompileGlslToSpv(
          shader_code.c_str(), shader_kind,
          utils::filesystem::GetName(asset_descr.asset_abs_path).c_str(),
          options)};

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
      Engine::Get().GetResourceManager().GetResourceFile(shader,
                                                         compression_mode_)};
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
