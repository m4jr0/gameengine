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

  shader.id = resource::GenerateResourceIdFromPath<resource::ShaderResource>(
      asset_descr.asset_path);
  shader.type_id = resource::ShaderResource::kResourceTypeId;

  shader.descr.shader_module_paths = Array<TString>{context.allocator};
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
    auto shader_file = nlohmann::json::parse(shader_context.file);

    resource_files.Reserve(1);

    shader.descr.rasterizer = GetRasterizerDescr(shader_file);
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
    auto raw_define_name_len{raw_define_name.size()};
    SetName(define, raw_define_name.c_str(), raw_define_name_len);

    if (!raw_define.contains(kCometEditorShaderKeyValue)) {
      continue;
    }

    const auto& raw_define_value_json{raw_define[kCometEditorShaderKeyValue]};
    const auto& raw_define_value{
        raw_define_value_json.get_ref<const nlohmann::json::string_t&>()};
    auto raw_define_value_len{raw_define_value.size()};
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
    auto raw_name_len{raw_name.size()};
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
      auto raw_field_name_len{raw_field_name.size()};
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
    auto raw_name_len{raw_name.size()};
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
      auto raw_field_name_len{raw_field_name.size()};
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

rendering::CullMode ShaderExporter::GetCullMode(
    std::string_view raw_cull_mode) {
  if (raw_cull_mode == kCometEditorShaderKeyRasterizerCullModeNone) {
    return rendering::CullMode::None;
  }

  if (raw_cull_mode == kCometEditorShaderKeyRasterizerCullModeFront) {
    return rendering::CullMode::Front;
  }

  if (raw_cull_mode == kCometEditorShaderKeyRasterizerCullModeBack) {
    return rendering::CullMode::Back;
  }

  if (raw_cull_mode == kCometEditorShaderKeyRasterizerCullModeFrontAndBack) {
    return rendering::CullMode::FrontAndBack;
  }

  COMET_LOG_GLOBAL_ERROR("Unknown or unsupported culling mode: ", raw_cull_mode,
                         "! Setting \"unknown\" mode instead.");
  return rendering::CullMode::Unknown;
}

rendering::RasterizerDescr ShaderExporter::GetRasterizerDescr(
    const nlohmann::json& shader_file) {
  rendering::RasterizerDescr rasterizer{};

  if (!shader_file.contains(kCometEditorShaderKeyRasterizer)) {
    rasterizer.is_wireframe = false;
    rasterizer.is_depth_bias = false;
    rasterizer.cull_mode = rendering::CullMode::Back;
    return rasterizer;
  }

  const auto& raw_rasterizer{shader_file[kCometEditorShaderKeyRasterizer]};

  if (!raw_rasterizer.is_object()) {
    COMET_LOG_GLOBAL_ERROR(
        "Wrong type found for rasterizer object: ", raw_rasterizer.type_name(),
        "! Using default rasterizer settings instead.");
    rasterizer.is_wireframe = false;
    rasterizer.is_depth_bias = false;
    rasterizer.cull_mode = rendering::CullMode::Back;
    return rasterizer;
  }

  rasterizer.is_wireframe =
      raw_rasterizer.value(kCometEditorShaderKeyRasterizerIsWireframe, false);

  rasterizer.is_depth_bias =
      raw_rasterizer.value(kCometEditorShaderKeyRasterizerIsDepthBias, false);

  rasterizer.cull_mode = GetCullMode(
      raw_rasterizer.value(kCometEditorShaderKeyRasterizerCullMode,
                           kCometEditorShaderKeyRasterizerCullModeBack));

  return rasterizer;
}

rendering::CompareOp ShaderExporter::GetCompareOp(
    std::string_view raw_compare_op) {
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpNever) {
    return rendering::CompareOp::Never;
  }
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpLess) {
    return rendering::CompareOp::Less;
  }
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpEqual) {
    return rendering::CompareOp::Equal;
  }
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpLessOrEqual) {
    return rendering::CompareOp::LessOrEqual;
  }
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpGreater) {
    return rendering::CompareOp::Greater;
  }
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpNotEqual) {
    return rendering::CompareOp::NotEqual;
  }
  if (raw_compare_op ==
      kCometEditorShaderKeyDepthStencilCompareOpGreaterOrEqual) {
    return rendering::CompareOp::GreaterOrEqual;
  }
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpAlways) {
    return rendering::CompareOp::Always;
  }

  COMET_LOG_GLOBAL_ERROR("Unknown or unsupported compare op: ", raw_compare_op,
                         "! Setting \"less\" compare op instead.");
  return rendering::CompareOp::Less;
}

rendering::DepthStencilDescr ShaderExporter::GetDepthStencilDescr(
    const nlohmann::json& shader_file) {
  rendering::DepthStencilDescr depth_stencil{};

  if (!shader_file.contains(kCometEditorShaderKeyDepthStencil)) {
    depth_stencil.is_depth_test = true;
    depth_stencil.is_depth_write = true;
    depth_stencil.compare_op = rendering::CompareOp::Less;
    return depth_stencil;
  }

  const auto& raw_depth_stencil{shader_file[kCometEditorShaderKeyDepthStencil]};

  if (!raw_depth_stencil.is_object()) {
    COMET_LOG_GLOBAL_ERROR("Wrong type found for depth_stencil object: ",
                           raw_depth_stencil.type_name(),
                           "! Using default depth-stencil settings instead.");
    depth_stencil.is_depth_test = true;
    depth_stencil.is_depth_write = true;
    depth_stencil.compare_op = rendering::CompareOp::Less;
    return depth_stencil;
  }

  depth_stencil.is_depth_test = raw_depth_stencil.value(
      kCometEditorShaderKeyDepthStencilIsDepthTest, true);

  depth_stencil.is_depth_write = raw_depth_stencil.value(
      kCometEditorShaderKeyDepthStencilIsDepthWrite, true);

  depth_stencil.compare_op = GetCompareOp(
      raw_depth_stencil.value(kCometEditorShaderKeyDepthStencilCompareOp,
                              kCometEditorShaderKeyDepthStencilCompareOpLess));

  return depth_stencil;
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

rendering::ShaderVertexLayout ShaderExporter::GetShaderVertexLayout(
    std::string_view raw_vertex_layout) {
  if (raw_vertex_layout == kCometEditorShaderKeyVertexLayoutNone) {
    return rendering::ShaderVertexLayout::None;
  }

  if (raw_vertex_layout == kCometEditorShaderKeyVertexLayoutSkinnedVertex) {
    return rendering::ShaderVertexLayout::SkinnedVertex;
  }

  if (raw_vertex_layout == kCometEditorShaderKeyVertexLayoutDebugLine) {
    return rendering::ShaderVertexLayout::DebugLine;
  }

  COMET_LOG_GLOBAL_ERROR(
      "Unknown or unsupported vertex layout: ", raw_vertex_layout,
      "! Setting \"none\" layout instead.");
  return rendering::ShaderVertexLayout::None;
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

  if (raw_data_type == kCometEditorShaderKeyVariableTypeSVec2) {
    return rendering::ShaderVariableType::S32Vec2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeSVec3) {
    return rendering::ShaderVariableType::S32Vec3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeSVec4) {
    return rendering::ShaderVariableType::S32Vec4;
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

  COMET_LOG_GLOBAL_ERROR("Unknown or unsupported shader field type: ",
                         raw_data_type, "! Setting \"unknown\" mode instead.");
  return rendering::ShaderVariableType::Unknown;
}

rendering::ShaderBindingType ShaderExporter::GetShaderBindingType(
    std::string_view raw_binding_type) {
  if (raw_binding_type == kCometEditorShaderKeyBindingTypeUniformBuffer) {
    return rendering::ShaderBindingType::UniformBuffer;
  }

  if (raw_binding_type == kCometEditorShaderKeyBindingTypeStorageBuffer) {
    return rendering::ShaderBindingType::StorageBuffer;
  }

  if (raw_binding_type ==
      kCometEditorShaderKeyBindingTypeCombinedImageSampler) {
    return rendering::ShaderBindingType::CombinedImageSampler;
  }

  if (raw_binding_type == kCometEditorShaderKeyBindingTypeSampledImage) {
    return rendering::ShaderBindingType::SampledImage;
  }

  if (raw_binding_type == kCometEditorShaderKeyBindingTypeSampler) {
    return rendering::ShaderBindingType::Sampler;
  }

  if (raw_binding_type == kCometEditorShaderKeyBindingTypeStorageImage) {
    return rendering::ShaderBindingType::StorageImage;
  }

  COMET_LOG_GLOBAL_ERROR(
      "Unknown or unsupported shader binding type: ", raw_binding_type,
      "! Setting \"unknown\" type instead.");
  return rendering::ShaderBindingType::Unknown;
}

rendering::ShaderBindingScope ShaderExporter::GetShaderBindingScope(
    std::string_view raw_binding_scope) {
  if (raw_binding_scope == kCometEditorShaderKeyBindingScopeGlobal) {
    return rendering::ShaderBindingScope::Global;
  }

  if (raw_binding_scope == kCometEditorShaderKeyBindingScopeMaterial) {
    return rendering::ShaderBindingScope::Material;
  }

  if (raw_binding_scope == kCometEditorShaderKeyBindingScopePass) {
    return rendering::ShaderBindingScope::Pass;
  }

  if (raw_binding_scope == kCometEditorShaderKeyBindingScopeDraw) {
    return rendering::ShaderBindingScope::Draw;
  }

  COMET_LOG_GLOBAL_ERROR(
      "Unknown or unsupported binding scope: ", raw_binding_scope,
      "! Setting \"unknown\" scope instead.");
  return rendering::ShaderBindingScope::Unknown;
}

rendering::ShaderMemoryLayout ShaderExporter::GetShaderMemoryLayout(
    std::string_view raw_layout) {
  if (raw_layout == kCometEditorShaderKeyMemoryLayoutStd140) {
    return rendering::ShaderMemoryLayout::Std140;
  }

  if (raw_layout == kCometEditorShaderKeyMemoryLayoutStd430) {
    return rendering::ShaderMemoryLayout::Std430;
  }

  if (raw_layout == kCometEditorShaderKeyMemoryLayoutPacked) {
    return rendering::ShaderMemoryLayout::Packed;
  }

  COMET_LOG_GLOBAL_ERROR("Unknown or unsupported memory layout: ", raw_layout,
                         "! Setting \"unknown\" layout instead.");
  return rendering::ShaderMemoryLayout::Unknown;
}

rendering::ShaderStageFlags ShaderExporter::GetShaderStageFlags(
    const nlohmann::json& raw_stages, memory::Allocator* allocator) {
  rendering::ShaderStageFlags stages{rendering::kShaderStageFlagBitsNone};

  if (!raw_stages.is_array()) {
    COMET_LOG_GLOBAL_ERROR("Wrong type found for stages array: ",
                           raw_stages.type_name(), "! Ignoring.");
    return stages;
  }

  auto raw_stages_list{
      nlohmann::json_to_array<std::string>(raw_stages, allocator)};

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

rendering::ShaderImageBindingSemantic
ShaderExporter::GetShaderImageBindingSemantic(std::string_view raw_semantic) {
  if (raw_semantic == kCometEditorShaderKeyImageSemanticMaterialDiffuse) {
    return rendering::ShaderImageBindingSemantic::MaterialDiffuse;
  }

  if (raw_semantic == kCometEditorShaderKeyImageSemanticMaterialSpecular) {
    return rendering::ShaderImageBindingSemantic::MaterialSpecular;
  }

  if (raw_semantic == kCometEditorShaderKeyImageSemanticMaterialNormal) {
    return rendering::ShaderImageBindingSemantic::MaterialNormal;
  }

  if (raw_semantic == kCometEditorShaderKeyImageSemanticMaterialTextures) {
    return rendering::ShaderImageBindingSemantic::MaterialTextures;
  }

  if (raw_semantic == kCometEditorShaderKeyImageSemanticMainShadowMap) {
    return rendering::ShaderImageBindingSemantic::MainShadowMap;
  }

  if (raw_semantic == kCometEditorShaderKeyImageSemanticShadowMaps) {
    return rendering::ShaderImageBindingSemantic::ShadowMaps;
  }

  return rendering::ShaderImageBindingSemantic::Unknown;
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