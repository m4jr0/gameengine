// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "shader_exporter.h"

#include "nlohmann/json.hpp"

#include "comet/core/file_system.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace editor {
namespace asset {
bool ShaderExporter::IsCompatible(std::string_view extension) const {
  return extension == "cshader";
}

std::vector<resource::ResourceFile> ShaderExporter::GetResourceFiles(
    AssetDescr& asset_descr) const {
  resource::ShaderResource shader{};
  shader.id = resource::GenerateResourceIdFromPath(asset_descr.asset_path);
  shader.type_id = resource::ShaderResource::kResourceTypeId;
  std::vector<resource::ResourceFile> to_return{};

  try {
    std::string metadata_raw;
    ReadStrFromFile(asset_descr.asset_abs_path, metadata_raw);

    if (metadata_raw.size() == 0) {
      return to_return;
    }

    // Every time we get an object, we must use assignment to prevent a bug with
    // GCC where the generated
    // type is an array (which is wrong).
    const auto shader_file = nlohmann::json::parse(metadata_raw);
    to_return.reserve(1);
    shader.descr.is_wireframe =
        shader_file.value(kCometEditorShaderKeyIsWireframe, false);
    shader.descr.cull_mode = GetCullMode(shader_file.value(
        kCometEditorShaderKeyCullMode, kCometEditorShaderCullModeNone));
    shader.descr.shader_module_paths = shader_file.value(
        kCometEditorShaderKeyShaderModulePaths, std::vector<std::string>{});

    if (shader_file.contains(kCometEditorShaderKeyVertexAttributes)) {
      const auto raw_vertex_attributes =
          shader_file[kCometEditorShaderKeyVertexAttributes];
      shader.descr.vertex_attributes.reserve(raw_vertex_attributes.size());

      for (uindex i{0}; i < raw_vertex_attributes.size(); ++i) {
        const auto& raw_vertex_attribute = raw_vertex_attributes[i];
        rendering::ShaderVertexAttributeDescr vertex_attribute{};
        vertex_attribute.name =
            raw_vertex_attribute[kCometEditorShaderKeyVertexAttributeName];

#ifdef COMET_GCC
        // Creating a string to keep GCC happy.
        vertex_attribute.type = GetShaderVertexAttributeType(std::string(
            raw_vertex_attribute[kCometEditorShaderKeyVertexAttributeType]));
#else
        vertex_attribute.type = GetShaderVertexAttributeType(
            raw_vertex_attribute[kCometEditorShaderKeyVertexAttributeType]);
#endif  // COMET_GCC
        shader.descr.vertex_attributes.push_back(std::move(vertex_attribute));
      }
    }

    if (shader_file.contains(kCometEditorShaderKeyUniforms)) {
      const auto raw_uniforms = shader_file[kCometEditorShaderKeyUniforms];
      shader.descr.uniforms.reserve(raw_uniforms.size());

      for (uindex i{0}; i < raw_uniforms.size(); ++i) {
        const auto& raw_uniform = raw_uniforms[i];
        rendering::ShaderUniformDescr uniform{};
        uniform.name = raw_uniform[kCometEditorShaderKeyUniformName];

#ifdef COMET_GCC
        // Creating a string to keep GCC happy.
        uniform.type = GetShaderUniformType(
            std::string(raw_uniform[kCometEditorShaderKeyUniformType]));
        uniform.scope = GetShaderUniformScope(
            std::string(raw_uniform[kCometEditorShaderKeyUniformScope]));
#else
        uniform.type =
            GetShaderUniformType(raw_uniform[kCometEditorShaderKeyUniformType]);
        uniform.scope = GetShaderUniformScope(
            raw_uniform[kCometEditorShaderKeyUniformScope]);
#endif  // COMET_GCC
        shader.descr.uniforms.push_back(std::move(uniform));
      }
    }
  } catch (const nlohmann::json::exception& error) {
    COMET_LOG_GLOBAL_ERROR("An error occurred while processing shader file: ",
                           error.what());
    return to_return;
  }

  to_return.push_back(resource::ResourceManager::Get().GetResourceFile(
      shader, compression_mode_));
  return to_return;
}

rendering::CullMode ShaderExporter::GetCullMode(
    std::string_view raw_cull_mode) {
  if (raw_cull_mode == kCometEditorShaderCullModeNone) {
    return rendering::CullMode::None;
  }

  if (raw_cull_mode == kCometEditorShaderCullModeFront) {
    return rendering::CullMode::Front;
  }

  if (raw_cull_mode == kCometEditorShaderCullModeBack) {
    return rendering::CullMode::Back;
  }

  if (raw_cull_mode == kCometEditorShaderCullModeFrontAndBack) {
    return rendering::CullMode::FrontAndBack;
  }

  COMET_LOG_GLOBAL_ERROR("Unknown or unsupported culling mode: ", raw_cull_mode,
                         "! Setting \"unknown\" mode instead.");

  return rendering::CullMode::Unknown;
}

rendering::ShaderVertexAttributeType
ShaderExporter::GetShaderVertexAttributeType(
    std::string_view raw_vertex_attribute_type) {
  if (raw_vertex_attribute_type == kCometEditorShaderAttributeTypeF16) {
    return rendering::ShaderVertexAttributeType::F16;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderAttributeTypeF32) {
    return rendering::ShaderVertexAttributeType::F32;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderAttributeTypeF64) {
    return rendering::ShaderVertexAttributeType::F64;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderAttributeTypeVec2) {
    return rendering::ShaderVertexAttributeType::Vec2;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderAttributeTypeVec3) {
    return rendering::ShaderVertexAttributeType::Vec3;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderAttributeTypeVec4) {
    return rendering::ShaderVertexAttributeType::Vec4;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderAttributeTypeS8) {
    return rendering::ShaderVertexAttributeType::S8;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderAttributeTypeS16) {
    return rendering::ShaderVertexAttributeType::S16;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderAttributeTypeS32) {
    return rendering::ShaderVertexAttributeType::S32;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderAttributeTypeU8) {
    return rendering::ShaderVertexAttributeType::U8;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderAttributeTypeU16) {
    return rendering::ShaderVertexAttributeType::U16;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderAttributeTypeU32) {
    return rendering::ShaderVertexAttributeType::U32;
  }

  COMET_LOG_GLOBAL_ERROR("Unknown or unsupported vertex attribute type: ",
                         raw_vertex_attribute_type,
                         "! Setting \"unknown\" mode instead.");

  return rendering::ShaderVertexAttributeType::Unknown;
}

rendering::ShaderUniformType ShaderExporter::GetShaderUniformType(
    std::string_view raw_data_type) {
  if (raw_data_type == kCometEditorShaderUniformTypeB32) {
    return rendering::ShaderUniformType::B32;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeS32) {
    return rendering::ShaderUniformType::S32;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeU32) {
    return rendering::ShaderUniformType::U32;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeF32) {
    return rendering::ShaderUniformType::F32;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeF64) {
    return rendering::ShaderUniformType::F64;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeB32Vec2) {
    return rendering::ShaderUniformType::B32Vec2;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeB32Vec3) {
    return rendering::ShaderUniformType::B32Vec3;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeB32Vec4) {
    return rendering::ShaderUniformType::B32Vec4;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeS32Vec2) {
    return rendering::ShaderUniformType::S32Vec2;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeS32Vec3) {
    return rendering::ShaderUniformType::S32Vec3;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeS32Vec4) {
    return rendering::ShaderUniformType::S32Vec4;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeU32Vec2) {
    return rendering::ShaderUniformType::U32Vec2;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeU32Vec3) {
    return rendering::ShaderUniformType::U32Vec3;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeU32Vec4) {
    return rendering::ShaderUniformType::U32Vec4;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeVec2) {
    return rendering::ShaderUniformType::Vec2;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeVec3) {
    return rendering::ShaderUniformType::Vec3;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeVec4) {
    return rendering::ShaderUniformType::Vec4;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeF64Vec2) {
    return rendering::ShaderUniformType::F64Vec2;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeF64Vec3) {
    return rendering::ShaderUniformType::F64Vec3;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeF64Vec4) {
    return rendering::ShaderUniformType::F64Vec4;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeMat2 ||
      raw_data_type == kCometEditorShaderUniformTypeMat2x2) {
    return rendering::ShaderUniformType::Mat2x2;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeMat2x3) {
    return rendering::ShaderUniformType::Mat2x3;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeMat2x4) {
    return rendering::ShaderUniformType::Mat2x4;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeMat3x2) {
    return rendering::ShaderUniformType::Mat3x2;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeMat3 ||
      raw_data_type == kCometEditorShaderUniformTypeMat3x3) {
    return rendering::ShaderUniformType::Mat3x3;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeMat3x4) {
    return rendering::ShaderUniformType::Mat3x4;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeMat4x2) {
    return rendering::ShaderUniformType::Mat4x2;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeMat4x3) {
    return rendering::ShaderUniformType::Mat4x3;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeMat4 ||
      raw_data_type == kCometEditorShaderUniformTypeMat4x4) {
    return rendering::ShaderUniformType::Mat4x4;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeSampler) {
    return rendering::ShaderUniformType::Sampler;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeImage) {
    return rendering::ShaderUniformType::Image;
  }

  if (raw_data_type == kCometEditorShaderUniformTypeAtomic) {
    return rendering::ShaderUniformType::Atomic;
  }

  COMET_LOG_GLOBAL_ERROR("Unknown or unsupported shader uniform type: ",
                         raw_data_type, "! Setting \"unknown\" mode instead.");

  return rendering::ShaderUniformType::Unknown;
}

rendering::ShaderUniformScope ShaderExporter::GetShaderUniformScope(
    std::string_view raw_uniform_scope) {
  if (raw_uniform_scope == kCometEditorShaderUniformScopeGlobal) {
    return rendering::ShaderUniformScope::Global;
  }

  if (raw_uniform_scope == kCometEditorShaderUniformScopeInstance) {
    return rendering::ShaderUniformScope::Instance;
  }

  if (raw_uniform_scope == kCometEditorShaderUniformScopeLocal) {
    return rendering::ShaderUniformScope::Local;
  }

  COMET_LOG_GLOBAL_ERROR(
      "Unknown or unsupported uniform scope: ", raw_uniform_scope,
      "! Setting \"unknown\" mode instead.");

  return rendering::ShaderUniformScope::Unknown;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
