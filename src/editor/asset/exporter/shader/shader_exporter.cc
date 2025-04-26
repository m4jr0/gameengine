// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "shader_exporter.h"

#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/file_system/file_system.h"
#include "comet/resource/resource.h"
#include "comet/resource/resource_manager.h"
#include "editor/asset/asset_utils.h"

namespace comet {
namespace editor {
namespace asset {
bool ShaderExporter::IsCompatible(CTStringView extension) const {
  return extension == COMET_TCHAR("cshader");
}

void ShaderExporter::PopulateFiles(ResourceFilesContext& context) const {
  resource::ShaderResource shader{};
  auto& asset_descr{context.asset_descr};
  shader.id = resource::GenerateResourceIdFromPath(asset_descr.asset_path);
  shader.type_id = resource::ShaderResource::kResourceTypeId;
  shader.descr.shader_module_paths = Array<TString>{context.allocator};
  shader.descr.vertex_attributes =
      Array<rendering::ShaderVertexAttributeDescr>{context.allocator};
  shader.descr.uniforms =
      Array<rendering::ShaderUniformDescr>{context.allocator};
  shader.descr.constants =
      Array<rendering::ShaderConstantDescr>{context.allocator};
  shader.descr.storages =
      Array<rendering::ShaderStorageDescr>{context.allocator};

  auto& resource_files{context.files};

  ShaderContext shader_context{};
  shader_context.asset_abs_path = asset_descr.asset_abs_path.GetCTStr();
  auto& scheduler{job::Scheduler::Get()};
  auto* size_counter{scheduler.GenerateCounter()};

  scheduler.KickAndWait(job::GenerateIOJobDescr(OnShaderSizeRequest,
                                                &shader_context, size_counter));

  scheduler.DestroyCounter(size_counter);

  // Add 1 extra character for the null terminator.
  shader_context.file_buffer_len = shader_context.file_len + 1;

  shader_context.file = static_cast<schar*>(COMET_FRAME_ALLOC_ALIGNED(
      shader_context.file_buffer_len, alignof(schar)));

  auto* load_counter{scheduler.GenerateCounter()};

  scheduler.KickAndWait(
      job::GenerateIOJobDescr(OnShaderLoading, &shader_context, load_counter));

  scheduler.DestroyCounter(load_counter);

  if (shader_context.file_len == 0) {
    return;
  }

  COMET_LOG_GLOBAL_DEBUG("Processing shader at ", shader_context.asset_abs_path,
                         "...");

  try {
    // Every time we get an object, we must use assignment to prevent a bug with
    // GCC where the generated type is an array (which is wrong).
    const auto shader_file = nlohmann::json::parse(shader_context.file);
    resource_files.Reserve(1);
    shader.descr.is_wireframe =
        shader_file.value(kCometEditorShaderKeyIsWireframe, false);
    shader.descr.cull_mode = GetCullMode(shader_file.value(
        kCometEditorShaderKeyCullMode, kCometEditorShaderKeyCullModeNone));
    shader.descr.topology = GetPrimitiveTopology(shader_file.value(
        kCometEditorShaderKeyTopology, kCometEditorShaderKeyTopologyTriangles));

    DumpShaderModules(shader_file, shader);
    DumpDefines(shader_file, context.allocator, shader);
    DumpVertexAttributes(shader_file, shader);
    DumpUniforms(shader_file, shader);
    DumpConstants(shader_file, shader);
    DumpStorages(shader_file, context.allocator, shader);

  } catch (const nlohmann::json::exception& error) {
    COMET_LOG_GLOBAL_ERROR("An error occurred while processing shader file: ",
                           error.what());
    return;
  }

  resource_files.PushBack(resource::ResourceManager::Get().GetResourceFile(
      shader, compression_mode_));

  COMET_LOG_GLOBAL_DEBUG("Shader processed at ", shader_context.asset_abs_path,
                         "...");
}

void ShaderExporter::DumpShaderModules(const nlohmann::json& shader_file,
                                       resource::ShaderResource& shader) {
  const auto& raw_module_paths{
      shader_file[kCometEditorShaderKeyShaderModulePaths]};
  shader.descr.shader_module_paths.Reserve(raw_module_paths.size());

  for (const auto& raw_module_path : raw_module_paths) {
    const auto& raw_module_path_str{
        raw_module_path.get_ref<const nlohmann::json::string_t&>()};
    shader.descr.shader_module_paths.PushBack(
        TString{GetTmpTChar(raw_module_path_str.c_str())});
  }
}

void ShaderExporter::DumpDefines(const nlohmann::json& shader_file,
                                 memory::Allocator* allocator,
                                 resource::ShaderResource& shader) {
  shader.descr.defines = Array<rendering::ShaderDefineDescr>{allocator};

  if (shader_file.contains(kCometEditorShaderKeyDefines)) {
    const auto& raw_defines = shader_file[kCometEditorShaderKeyDefines];
    shader.descr.defines.Reserve(raw_defines.size());

    for (usize j{0}; j < raw_defines.size(); ++j) {
      const auto& raw_define = raw_defines[j];
      auto& define{shader.descr.defines.EmplaceBack()};

      const auto& raw_define_name_json{
          raw_define[kCometEditorShaderKeyDefineName]};
      const auto& raw_define_name{
          raw_define_name_json.get_ref<const nlohmann::json::string_t&>()};
      auto raw_define_name_len{raw_define_name.size()};
      SetName(define, raw_define_name.c_str(), raw_define_name_len);

      if (!raw_define.contains(kCometEditorShaderKeyDefineValue)) {
        continue;
      }

      const auto& raw_define_value_json{
          raw_define[kCometEditorShaderKeyDefineValue]};
      const auto& raw_define_value{
          raw_define_value_json.get_ref<const nlohmann::json::string_t&>()};
      auto raw_define_value_len{raw_define_value.size()};
      SetValue(define, raw_define_value.c_str(), raw_define_value_len);
    }
  }
}

void ShaderExporter::DumpVertexAttributes(const nlohmann::json& shader_file,
                                          resource::ShaderResource& shader) {
  if (!shader_file.contains(kCometEditorShaderKeyVertexAttributes)) {
    return;
  }

  // Every time we get an object, we must use assignment to prevent a bug with
  // GCC where the generated type is an array (which is wrong).
  const auto& raw_vertex_attributes =
      shader_file[kCometEditorShaderKeyVertexAttributes];
  shader.descr.vertex_attributes.Reserve(raw_vertex_attributes.size());

  for (usize i{0}; i < raw_vertex_attributes.size(); ++i) {
    const auto& raw_vertex_attribute = raw_vertex_attributes[i];
    auto& vertex_attribute{shader.descr.vertex_attributes.EmplaceBack()};

    const auto& raw_vertex_attribute_name_json{
        raw_vertex_attribute[kCometEditorShaderKeyVertexAttributeName]};
    const auto& raw_vertex_attribute_name{
        raw_vertex_attribute_name_json
            .get_ref<const nlohmann::json::string_t&>()};
    auto raw_vertex_attribute_name_len{raw_vertex_attribute_name.size()};
    SetName(vertex_attribute, raw_vertex_attribute_name.c_str(),
            raw_vertex_attribute_name_len);

#ifdef COMET_GCC
    // Keep GCC happy.
    const auto& raw_vertex_attribute_type_json{
        raw_vertex_attribute[kCometEditorShaderKeyVertexAttributeType]};
    const auto& raw_vertex_attribute_type{
        raw_vertex_attribute_type_json
            .get_ref<const nlohmann::json::string_t&>()};

    vertex_attribute.type =
        GetShaderVertexAttributeType(raw_vertex_attribute_type.c_str());
#else
    vertex_attribute.type = GetShaderVertexAttributeType(
        raw_vertex_attribute[kCometEditorShaderKeyVertexAttributeType]);
#endif  // COMET_GCC
  }
}

void ShaderExporter::DumpUniforms(const nlohmann::json& shader_file,
                                  resource::ShaderResource& shader) {
  if (!shader_file.contains(kCometEditorShaderKeyUniforms)) {
    return;
  }

  // Every time we get an object, we must use assignment to prevent a bug with
  // GCC where the generated type is an array (which is wrong).
  const auto& raw_uniforms = shader_file[kCometEditorShaderKeyUniforms];
  shader.descr.uniforms.Reserve(raw_uniforms.size());

  for (usize i{0}; i < raw_uniforms.size(); ++i) {
    const auto& raw_uniform = raw_uniforms[i];
    auto& uniform{shader.descr.uniforms.EmplaceBack()};

    const auto& raw_uniform_name_json{
        raw_uniform[kCometEditorShaderKeyVertexAttributeName]};
    const auto& raw_uniform_name{
        raw_uniform_name_json.get_ref<const nlohmann::json::string_t&>()};
    auto raw_uniform_name_len{raw_uniform_name.size()};
    SetName(uniform, raw_uniform_name.c_str(), raw_uniform_name_len);

#ifdef COMET_GCC
    // Keep GCC happy.
    const auto& raw_uniform_type_json{
        raw_uniform[kCometEditorShaderKeyUniformType]};
    const auto& raw_uniform_type{
        raw_uniform_type_json.get_ref<const nlohmann::json::string_t&>()};
    uniform.type = GetShaderVariableType(raw_uniform_type.c_str());

    const auto& raw_uniform_scope_json{
        raw_uniform[kCometEditorShaderKeyUniformScope]};
    const auto& raw_uniform_scope{
        raw_uniform_scope_json.get_ref<const nlohmann::json::string_t&>()};
    uniform.scope = GetShaderUniformScope(raw_uniform_scope.c_str());
#else
    uniform.type =
        GetShaderVariableType(raw_uniform[kCometEditorShaderKeyUniformType]);
    uniform.scope =
        GetShaderUniformScope(raw_uniform[kCometEditorShaderKeyUniformScope]);
#endif  // COMET_GCC

    uniform.stages =
        GetShaderStageFlags(raw_uniform[kCometEditorShaderKeyUniformStages]);
  }
}

void ShaderExporter::DumpConstants(const nlohmann::json& shader_file,
                                   resource::ShaderResource& shader) {
  if (!shader_file.contains(kCometEditorShaderKeyConstants)) {
    return;
  }

  // Every time we get an object, we must use assignment to prevent a bug with
  // GCC where the generated type is an array (which is wrong).
  const auto& raw_constants = shader_file[kCometEditorShaderKeyConstants];
  shader.descr.constants.Reserve(raw_constants.size());

  for (usize i{0}; i < raw_constants.size(); ++i) {
    const auto& raw_constant{raw_constants[i]};
    auto& constant{shader.descr.constants.EmplaceBack()};

    const auto& constant_name_json{
        raw_constant[kCometEditorShaderKeyStorageName]};
    const auto& constant_name{
        constant_name_json.get_ref<const nlohmann::json::string_t&>()};
    auto constant_name_len{constant_name.size()};
    SetName(constant, constant_name.c_str(), constant_name_len);

    constant.stages =
        GetShaderStageFlags(raw_constant[kCometEditorShaderKeyStorageStages]);

#ifdef COMET_GCC
    // Keep GCC happy.
    const auto& raw_constant_type_json{
        raw_constant[kCometEditorShaderKeyUniformType]};
    const auto& raw_constant_type{
        raw_constant_type_json.get_ref<const nlohmann::json::string_t&>()};
    constant.type = GetShaderVariableType(raw_constant_type.c_str());
#else
    constant.type =
        GetShaderVariableType(raw_constant[kCometEditorShaderKeyUniformType]);
#endif  // COMET_GCC
  }
}

void ShaderExporter::DumpStorages(const nlohmann::json& shader_file,
                                  memory::Allocator* allocator,
                                  resource::ShaderResource& shader) {
  if (!shader_file.contains(kCometEditorShaderKeyStorages)) {
    return;
  }

  // Every time we get an object, we must use assignment to prevent a bug with
  // GCC where the generated type is an array (which is wrong).
  const auto& raw_storages = shader_file[kCometEditorShaderKeyStorages];
  shader.descr.storages.Reserve(raw_storages.size());

  for (usize i{0}; i < raw_storages.size(); ++i) {
    const auto& raw_storage{raw_storages[i]};
    auto& storage{shader.descr.storages.EmplaceBack()};

    const auto& storage_name_json{
        raw_storage[kCometEditorShaderKeyStorageName]};
    const auto& storage_name{
        storage_name_json.get_ref<const nlohmann::json::string_t&>()};
    auto storage_name_len{storage_name.size()};
    SetName(storage, storage_name.c_str(), storage_name_len);

    storage.stages =
        GetShaderStageFlags(raw_storage[kCometEditorShaderKeyStorageStages]);

    storage.properties =
        Array<rendering::ShaderStoragePropertyDescr>{allocator};

    if (raw_storage.contains(kCometEditorShaderKeyStorageLayout)) {
      const auto& raw_properties =
          raw_storage[kCometEditorShaderKeyStorageLayout];
      storage.properties.Reserve(raw_properties.size());

      for (usize j{0}; j < raw_properties.size(); ++j) {
        const auto& raw_property = raw_properties[j];
        auto& property{storage.properties.EmplaceBack()};

        const auto& raw_property_name_json{
            raw_property[kCometEditorShaderKeyStoragePropertyName]};
        const auto& raw_property_name{
            raw_property_name_json.get_ref<const nlohmann::json::string_t&>()};
        auto raw_property_name_len{raw_property_name.size()};
        SetName(property, raw_property_name.c_str(), raw_property_name_len);

#ifdef COMET_GCC
        // Keep GCC happy.
        const auto& raw_property_type_json{
            raw_property[kCometEditorShaderKeyStoragePropertyType]};
        const auto& raw_property_type{
            raw_property_type_json.get_ref<const nlohmann::json::string_t&>()};
        property.type = GetShaderVariableType(raw_property_type.c_str());
#else
        property.type = GetShaderVariableType(
            raw_property[kCometEditorShaderKeyStoragePropertyType]);
#endif  // COMET_GCC
      }
    }

    if (raw_storage.contains(kCometEditorShaderKeyStorageEngineDefine)) {
      const auto& raw_engine_define_name_json{
          raw_storage[kCometEditorShaderKeyStorageEngineDefine]};
      const auto& raw_engine_define_name{
          raw_engine_define_name_json
              .get_ref<const nlohmann::json::string_t&>()};
      auto raw_engine_define_name_len{raw_engine_define_name.size()};
      SetEngineDefine(storage, raw_engine_define_name.c_str(),
                      raw_engine_define_name_len);
    }
  }
}

rendering::CullMode ShaderExporter::GetCullMode(
    std::string_view raw_cull_mode) {
  if (raw_cull_mode == kCometEditorShaderKeyCullModeNone) {
    return rendering::CullMode::None;
  }

  if (raw_cull_mode == kCometEditorShaderKeyCullModeFront) {
    return rendering::CullMode::Front;
  }

  if (raw_cull_mode == kCometEditorShaderKeyCullModeBack) {
    return rendering::CullMode::Back;
  }

  if (raw_cull_mode == kCometEditorShaderKeyCullModeFrontAndBack) {
    return rendering::CullMode::FrontAndBack;
  }

  COMET_LOG_GLOBAL_ERROR("Unknown or unsupported culling mode: ", raw_cull_mode,
                         "! Setting \"unknown\" mode instead.");

  return rendering::CullMode::Unknown;
}

rendering::PrimitiveTopology ShaderExporter::GetPrimitiveTopology(
    std::string_view raw_topology) {
  if (raw_topology == kCometEditorShaderKeyTopologyPoints) {
    return rendering::PrimitiveTopology::Points;
  }

  if (raw_topology == kCometEditorShaderKeyTopologyLines) {
    return rendering::PrimitiveTopology::Lines;
  }

  if (raw_topology == kCometEditorShaderKeyTopologyLineStrip) {
    return rendering::PrimitiveTopology::LineStrip;
  }

  if (raw_topology == kCometEditorShaderKeyTopologyTriangles) {
    return rendering::PrimitiveTopology::Triangles;
  }

  if (raw_topology == kCometEditorShaderKeyTopologyTriangleStrip) {
    return rendering::PrimitiveTopology::TriangleStrip;
  }

  COMET_LOG_GLOBAL_ERROR("Unknown or unsupported topology: ", raw_topology,
                         "! Setting \"unknown\" mode instead.");

  return rendering::PrimitiveTopology::Unknown;
}

rendering::ShaderVertexAttributeType
ShaderExporter::GetShaderVertexAttributeType(
    std::string_view raw_vertex_attribute_type) {
  if (raw_vertex_attribute_type == kCometEditorShaderKeyAttributeTypeF16) {
    return rendering::ShaderVertexAttributeType::F16;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderKeyAttributeTypeF32) {
    return rendering::ShaderVertexAttributeType::F32;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderKeyAttributeTypeF64) {
    return rendering::ShaderVertexAttributeType::F64;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderKeyAttributeTypeVec2) {
    return rendering::ShaderVertexAttributeType::Vec2;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderKeyAttributeTypeVec3) {
    return rendering::ShaderVertexAttributeType::Vec3;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderKeyAttributeTypeVec4) {
    return rendering::ShaderVertexAttributeType::Vec4;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderKeyAttributeTypeS8) {
    return rendering::ShaderVertexAttributeType::S8;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderKeyAttributeTypeS16) {
    return rendering::ShaderVertexAttributeType::S16;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderKeyAttributeTypeS32) {
    return rendering::ShaderVertexAttributeType::S32;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderKeyAttributeTypeU8) {
    return rendering::ShaderVertexAttributeType::U8;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderKeyAttributeTypeU16) {
    return rendering::ShaderVertexAttributeType::U16;
  }

  if (raw_vertex_attribute_type == kCometEditorShaderKeyAttributeTypeU32) {
    return rendering::ShaderVertexAttributeType::U32;
  }

  COMET_LOG_GLOBAL_ERROR("Unknown or unsupported vertex attribute type: ",
                         raw_vertex_attribute_type,
                         "! Setting \"unknown\" mode instead.");

  return rendering::ShaderVertexAttributeType::Unknown;
}

rendering::ShaderVariableType ShaderExporter::GetShaderVariableType(
    std::string_view raw_data_type) {
  if (raw_data_type == kCometEditorShaderKeyVariableTypeB32) {
    return rendering::ShaderVariableType::B32;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeS32) {
    return rendering::ShaderVariableType::S32;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeU32) {
    return rendering::ShaderVariableType::U32;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeF32) {
    return rendering::ShaderVariableType::F32;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeF64) {
    return rendering::ShaderVariableType::F64;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeB32Vec2) {
    return rendering::ShaderVariableType::B32Vec2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeB32Vec3) {
    return rendering::ShaderVariableType::B32Vec3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeB32Vec4) {
    return rendering::ShaderVariableType::B32Vec4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeS32Vec2) {
    return rendering::ShaderVariableType::S32Vec2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeS32Vec3) {
    return rendering::ShaderVariableType::S32Vec3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeS32Vec4) {
    return rendering::ShaderVariableType::S32Vec4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeU32Vec2) {
    return rendering::ShaderVariableType::U32Vec2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeU32Vec3) {
    return rendering::ShaderVariableType::U32Vec3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeU32Vec4) {
    return rendering::ShaderVariableType::U32Vec4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeVec2) {
    return rendering::ShaderVariableType::Vec2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeVec3) {
    return rendering::ShaderVariableType::Vec3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeVec4) {
    return rendering::ShaderVariableType::Vec4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeF64Vec2) {
    return rendering::ShaderVariableType::F64Vec2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeF64Vec3) {
    return rendering::ShaderVariableType::F64Vec3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeF64Vec4) {
    return rendering::ShaderVariableType::F64Vec4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat2 ||
      raw_data_type == kCometEditorShaderKeyVariableTypeMat2x2) {
    return rendering::ShaderVariableType::Mat2x2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat2x3) {
    return rendering::ShaderVariableType::Mat2x3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat2x4) {
    return rendering::ShaderVariableType::Mat2x4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat3x2) {
    return rendering::ShaderVariableType::Mat3x2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat3 ||
      raw_data_type == kCometEditorShaderKeyVariableTypeMat3x3) {
    return rendering::ShaderVariableType::Mat3x3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat3x4) {
    return rendering::ShaderVariableType::Mat3x4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat4x2) {
    return rendering::ShaderVariableType::Mat4x2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat4x3) {
    return rendering::ShaderVariableType::Mat4x3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat4 ||
      raw_data_type == kCometEditorShaderKeyVariableTypeMat4x4) {
    return rendering::ShaderVariableType::Mat4x4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeSampler) {
    return rendering::ShaderVariableType::Sampler;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeImage) {
    return rendering::ShaderVariableType::Image;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeAtomic) {
    return rendering::ShaderVariableType::Atomic;
  }

  COMET_LOG_GLOBAL_ERROR("Unknown or unsupported shader uniform type: ",
                         raw_data_type, "! Setting \"unknown\" mode instead.");

  return rendering::ShaderVariableType::Unknown;
}

rendering::ShaderUniformScope ShaderExporter::GetShaderUniformScope(
    std::string_view raw_uniform_scope) {
  if (raw_uniform_scope == kCometEditorShaderKeyUniformScopeGlobal) {
    return rendering::ShaderUniformScope::Global;
  }

  if (raw_uniform_scope == kCometEditorShaderKeyUniformScopeInstance) {
    return rendering::ShaderUniformScope::Instance;
  }

  COMET_LOG_GLOBAL_ERROR(
      "Unknown or unsupported uniform scope: ", raw_uniform_scope,
      "! Setting \"unknown\" mode instead.");

  return rendering::ShaderUniformScope::Unknown;
}

rendering::ShaderStageFlags ShaderExporter::GetShaderStageFlags(
    const nlohmann::json& raw_stages) {
  rendering::ShaderStageFlags stages{rendering::kShaderStageFlagBitsNone};

  if (!raw_stages.is_array()) {
    COMET_LOG_GLOBAL_ERROR("Wrong type found for stages array: ",
                           raw_stages.type_name(), "! Ignoring.");
    return stages;
  }

  auto raw_stages_list{raw_stages.get<frame::FrameArray<std::string>>()};

  for (const auto& raw_stage : raw_stages_list) {
    if (raw_stage == kCometEditorShaderKeyStageCompute) {
      stages |= rendering::kShaderStageFlagBitsCompute;
    } else if (raw_stage == kCometEditorShaderKeyStageVertex) {
      stages |= rendering::kShaderStageFlagBitsVertex;
    } else if (raw_stage == kCometEditorShaderKeyStageFragment) {
      stages |= rendering::kShaderStageFlagBitsFragment;
    } else {
      COMET_LOG_GLOBAL_ERROR(
          "Unknown or unsupported stage provided: ", raw_stage, "! Ignoring.");
    }
  }

  return stages;
}

void ShaderExporter::OnShaderSizeRequest(job::IOJobParamsHandle params_handle) {
  auto* shader_context{static_cast<ShaderContext*>(params_handle)};
  shader_context->file_len = GetSize(shader_context->asset_abs_path);
}

void ShaderExporter::OnShaderLoading(job::IOJobParamsHandle params_handle) {
  auto* shader_context{static_cast<ShaderContext*>(params_handle)};
  ReadStrFromFile(shader_context->asset_abs_path, shader_context->file,
                  shader_context->file_buffer_len, &shader_context->file_len);
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
