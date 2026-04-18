// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "shader_exporter.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include "nlohmann/json.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/type/array.h"
#include "comet/rendering/rendering_utils.h"
#include "comet/resource/resource.h"
#include "comet/resource/resource_manager.h"
#include "editor/asset/exporter/shader/data/shader_export_keys.h"
#include "editor/asset/exporter/shader/utils/shader_export_utils.h"

namespace comet {
namespace editor {
namespace asset {
bool ShaderExporter::IsCompatible(CTStringView extension) const {
  return extension == COMET_TCHAR("cshader");
}

void ShaderExporter::PopulateFiles(ResourceFilesContext& context) const {
  resource::ShaderResource shader{};
  auto& asset_descr{context.asset_descr};

  const auto shader_resource_id{
      resource::GenerateResourceIdFromPath<resource::ShaderResource>(
          asset_descr.asset_path)};
  shader.id = shader_resource_id.GetValue();
  shader.type_id = resource::ShaderResource::kResourceTypeId;

  shader.descr.shader_module_resource_ids =
      Array<resource::ShaderModuleResourceId>{context.allocator};
  shader.descr.defines = Array<rendering::ShaderDefineDescr>{context.allocator};
  shader.descr.bindings =
      Array<rendering::ShaderBindingDescr>{context.allocator};
  shader.descr.push_constants =
      Array<rendering::ShaderPushConstantDescr>{context.allocator};

  auto& resource_files{context.files};

  ShaderContext shader_context{};
  shader_context.asset_abs_path = asset_descr.asset_abs_path.GetCTStr();

  auto& scheduler{job::Scheduler::Get()};
  {
    job::CounterGuard size_guard{};
    scheduler.KickAndWait(job::GenerateIOJobDescr(
        OnShaderSizeRequest, &shader_context, size_guard.GetCounter()));
  }

  shader_context.file_buffer_len = shader_context.file_len + 1;

  auto* allocator{context.allocator};
  shader_context.file =
      allocator->AllocateMany<schar>(shader_context.file_buffer_len);

  {
    job::CounterGuard load_guard{};
    scheduler.KickAndWait(job::GenerateIOJobDescr(
        OnShaderLoading, &shader_context, load_guard.GetCounter()));
  }

  if (shader_context.file_len == 0) {
    allocator->Deallocate(shader_context.file);
    return;
  }

  COMET_LOG_GLOBAL_DEBUG("Processing shader at ", shader_context.asset_abs_path,
                         "...");

  try {
    // Every time we get an object, we must use assignment to prevent a bug
    // with
    // GCC where the generated type is an array (which is wrong).
    const auto shader_file = nlohmann::json::parse(shader_context.file);
    resource_files.Reserve(1);

    shader.descr.rasterizer = GetRasterizerDescr(shader_file);
    shader.descr.depth_stencil = GetDepthStencilDescr(shader_file);

    shader.descr.topology = GetPrimitiveTopology(shader_file.value(
        kCometEditorShaderKeyTopology, kCometEditorShaderKeyTopologyTriangles));

    shader.descr.vertex_layout = GetShaderVertexLayout(
        shader_file.value(kCometEditorShaderKeyVertexLayout,
                          kCometEditorShaderKeyVertexLayoutNone));

    DumpShaderModules(shader_file, shader);
    DumpDefines(shader_file, context.allocator, shader);
    DumpBindings(shader_file, context.allocator, shader);
    DumpPushConstants(shader_file, context.allocator, shader);
  } catch (const nlohmann::json::exception& error) {
    COMET_LOG_GLOBAL_ERROR("An error occurred while processing shader file: ",
                           error.what());
    allocator->Deallocate(shader_context.file);
    return;
  }

  resource_files.PushBack(resource::ResourceManager::Get().GetShaders()->Pack(
      shader, compression_mode_));

  allocator->Deallocate(shader_context.file);

  COMET_LOG_GLOBAL_DEBUG("Shader processed at ", shader_context.asset_abs_path,
                         "...");
}

void ShaderExporter::DumpShaderModules(const nlohmann::json& shader_file,
                                       resource::ShaderResource& shader) {
  if (!shader_file.contains(kCometEditorShaderKeyShaderModulePaths)) {
    COMET_LOG_GLOBAL_ERROR("Shader has no shader modules defined!");
    return;
  }

  const auto& raw_module_paths{
      shader_file[kCometEditorShaderKeyShaderModulePaths]};

  if (!raw_module_paths.is_array()) {
    COMET_LOG_GLOBAL_ERROR("shader_module_paths must be an array!");
    return;
  }

  shader.descr.shader_module_resource_ids.Reserve(raw_module_paths.size());

  for (const auto& raw_module_path : raw_module_paths) {
    const auto& raw_module_path_str{
        raw_module_path.get_ref<const nlohmann::json::string_t&>()};

    const auto module_resource_id{
        resource::GenerateResourceIdFromPath<resource::ShaderModuleResource>(
            GetTmpTChar(raw_module_path_str.c_str()))};

    shader.descr.shader_module_resource_ids.PushBack(module_resource_id);
  }
}

void ShaderExporter::DumpDefines(const nlohmann::json& shader_file,
                                 memory::Allocator* allocator,
                                 resource::ShaderResource& shader) {
  shader.descr.defines = Array<rendering::ShaderDefineDescr>{allocator};

  if (!shader_file.contains(kCometEditorShaderKeyDefines)) {
    return;
  }

  const auto& raw_defines = shader_file[kCometEditorShaderKeyDefines];
  shader.descr.defines.Reserve(raw_defines.size());

  for (usize i{0}; i < raw_defines.size(); ++i) {
    const auto& raw_define = raw_defines[i];
    auto& define{shader.descr.defines.EmplaceBack()};

    const auto& raw_define_name_json{raw_define[kCometEditorShaderKeyName]};
    const auto& raw_define_name{
        raw_define_name_json.get_ref<const nlohmann::json::string_t&>()};
    const auto raw_define_name_len{raw_define_name.size()};
    SetName(define, raw_define_name.c_str(), raw_define_name_len);

    if (!raw_define.contains(kCometEditorShaderKeyValue)) {
      continue;
    }

    const auto& raw_define_value_json{raw_define[kCometEditorShaderKeyValue]};
    const auto& raw_define_value{
        raw_define_value_json.get_ref<const nlohmann::json::string_t&>()};
    const auto raw_define_value_len{raw_define_value.size()};
    SetValue(define, raw_define_value.c_str(), raw_define_value_len);
  }
}

void ShaderExporter::DumpBindings(const nlohmann::json& shader_file,
                                  memory::Allocator* allocator,
                                  resource::ShaderResource& shader) {
  if (!shader_file.contains(kCometEditorShaderKeyBindings)) {
    return;
  }

  // Every time we get an object, we must use assignment to prevent a bug
  // with
  // GCC where the generated type is an array (which is wrong).
  const auto& raw_bindings = shader_file[kCometEditorShaderKeyBindings];
  shader.descr.bindings.Reserve(raw_bindings.size());

  for (usize i{0}; i < raw_bindings.size(); ++i) {
    const auto& raw_binding = raw_bindings[i];
    auto& binding_descr{shader.descr.bindings.EmplaceBack()};

    const auto& raw_name_json{raw_binding[kCometEditorShaderKeyName]};
    const auto& raw_name{
        raw_name_json.get_ref<const nlohmann::json::string_t&>()};
    const auto raw_name_len{raw_name.size()};
    SetName(binding_descr, raw_name.c_str(), raw_name_len);

#ifdef COMET_GCC
    const auto& raw_type_json{raw_binding[kCometEditorShaderKeyType]};
    const auto& raw_type{
        raw_type_json.get_ref<const nlohmann::json::string_t&>()};
    binding_descr.type = GetShaderBindingType(raw_type.c_str());

    const auto& raw_scope_json{raw_binding[kCometEditorShaderKeyScope]};
    const auto& raw_scope{
        raw_scope_json.get_ref<const nlohmann::json::string_t&>()};
    binding_descr.scope = GetShaderBindingScope(raw_scope.c_str());
#else
    binding_descr.type =
        GetShaderBindingType(raw_binding[kCometEditorShaderKeyType]);
    binding_descr.scope =
        GetShaderBindingScope(raw_binding[kCometEditorShaderKeyScope]);
#endif  // COMET_GCC

    if (raw_binding.contains(kCometEditorShaderKeyLayout)) {
#ifdef COMET_GCC
      const auto& raw_layout_json{raw_binding[kCometEditorShaderKeyLayout]};
      const auto& raw_layout{
          raw_layout_json.get_ref<const nlohmann::json::string_t&>()};
      binding_descr.layout = GetShaderMemoryLayout(raw_layout.c_str());
#else
      binding_descr.layout =
          GetShaderMemoryLayout(raw_binding[kCometEditorShaderKeyLayout]);
#endif  // COMET_GCC
    } else {
      binding_descr.layout = rendering::ShaderMemoryLayout::Unknown;
    }

    binding_descr.stages = GetShaderStageFlags(
        raw_binding[kCometEditorShaderKeyStages], allocator);

    binding_descr.set = raw_binding.value(kCometEditorShaderKeySet, 0u);
    binding_descr.binding = raw_binding.value(kCometEditorShaderKeyBinding, 0u);
    binding_descr.descriptor_count =
        raw_binding.value(kCometEditorShaderKeyDescriptorCount, 1u);

    if (raw_binding.contains(kCometEditorShaderKeyBindingImageSemantic)) {
      const auto& raw_semantic_json{
          raw_binding[kCometEditorShaderKeyBindingImageSemantic]};
      const auto& raw_semantic{
          raw_semantic_json.get_ref<const nlohmann::json::string_t&>()};

      binding_descr.image_semantic =
          GetShaderImageBindingSemantic(raw_semantic.c_str());
    }

    binding_descr.fields = Array<rendering::ShaderFieldDescr>{allocator};

    if (!raw_binding.contains(kCometEditorShaderKeyFields)) {
      continue;
    }

    const auto& raw_fields{raw_binding[kCometEditorShaderKeyFields]};
    binding_descr.fields.Reserve(raw_fields.size());

    for (usize j{0}; j < raw_fields.size(); ++j) {
      const auto& raw_field = raw_fields[j];
      auto& field_descr{binding_descr.fields.EmplaceBack()};

      const auto& raw_field_name_json{raw_field[kCometEditorShaderKeyName]};
      const auto& raw_field_name{
          raw_field_name_json.get_ref<const nlohmann::json::string_t&>()};
      const auto raw_field_name_len{raw_field_name.size()};
      SetName(field_descr, raw_field_name.c_str(), raw_field_name_len);

#ifdef COMET_GCC
      const auto& raw_field_type_json{raw_field[kCometEditorShaderKeyType]};
      const auto& raw_field_type{
          raw_field_type_json.get_ref<const nlohmann::json::string_t&>()};
      field_descr.type = GetShaderVariableType(raw_field_type.c_str());
#else
      field_descr.type =
          GetShaderVariableType(raw_field[kCometEditorShaderKeyType]);
#endif  // COMET_GCC

      field_descr.array_count =
          raw_field.value(kCometEditorShaderKeyArrayCount, 1u);
    }
  }
}

void ShaderExporter::DumpPushConstants(const nlohmann::json& shader_file,
                                       memory::Allocator* allocator,
                                       resource::ShaderResource& shader) {
  if (!shader_file.contains(kCometEditorShaderKeyPushConstants)) {
    return;
  }

  const auto& raw_push_constants =
      shader_file[kCometEditorShaderKeyPushConstants];
  shader.descr.push_constants.Reserve(raw_push_constants.size());

  for (usize i{0}; i < raw_push_constants.size(); ++i) {
    const auto& raw_push_constant = raw_push_constants[i];
    auto& push_constant_descr{shader.descr.push_constants.EmplaceBack()};

    const auto& raw_name_json{raw_push_constant[kCometEditorShaderKeyName]};
    const auto& raw_name{
        raw_name_json.get_ref<const nlohmann::json::string_t&>()};
    const auto raw_name_len{raw_name.size()};
    SetName(push_constant_descr, raw_name.c_str(), raw_name_len);

    push_constant_descr.stages = GetShaderStageFlags(
        raw_push_constant[kCometEditorShaderKeyStages], allocator);

    push_constant_descr.fields = Array<rendering::ShaderFieldDescr>{allocator};

    if (!raw_push_constant.contains(kCometEditorShaderKeyFields)) {
      continue;
    }

    const auto& raw_fields{raw_push_constant[kCometEditorShaderKeyFields]};
    push_constant_descr.fields.Reserve(raw_fields.size());

    for (usize j{0}; j < raw_fields.size(); ++j) {
      const auto& raw_field = raw_fields[j];
      auto& field_descr{push_constant_descr.fields.EmplaceBack()};

      const auto& raw_field_name_json{raw_field[kCometEditorShaderKeyName]};
      const auto& raw_field_name{
          raw_field_name_json.get_ref<const nlohmann::json::string_t&>()};
      const auto raw_field_name_len{raw_field_name.size()};
      SetName(field_descr, raw_field_name.c_str(), raw_field_name_len);

#ifdef COMET_GCC
      const auto& raw_field_type_json{raw_field[kCometEditorShaderKeyType]};
      const auto& raw_field_type{
          raw_field_type_json.get_ref<const nlohmann::json::string_t&>()};
      field_descr.type = GetShaderVariableType(raw_field_type.c_str());
#else
      field_descr.type =
          GetShaderVariableType(raw_field[kCometEditorShaderKeyType]);
#endif  // COMET_GCC

      field_descr.array_count =
          raw_field.value(kCometEditorShaderKeyArrayCount, 1u);
    }
  }
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