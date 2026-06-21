// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "assetc_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "shader_export_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "asset_utils.h"
#include "exporter/shader/data/shader_export_keys.h"

namespace comet {
namespace tool {
namespace assetc {
render::CullMode GetCullMode(std::string_view raw_cull_mode) {
  if (raw_cull_mode == kCometEditorShaderKeyRasterizerCullModeNone) {
    return render::CullMode::None;
  }

  if (raw_cull_mode == kCometEditorShaderKeyRasterizerCullModeFront) {
    return render::CullMode::Front;
  }

  if (raw_cull_mode == kCometEditorShaderKeyRasterizerCullModeBack) {
    return render::CullMode::Back;
  }

  if (raw_cull_mode == kCometEditorShaderKeyRasterizerCullModeFrontAndBack) {
    return render::CullMode::FrontAndBack;
  }

  COMET_LOG_ERROR(LoggerType::External, "shader_export_utils::GetCullMode",
                  "cull mode is unsupported", "cull_mode", raw_cull_mode);
  return render::CullMode::Unknown;
}

render::RasterizerDescr GetRasterizerDescr(
    const nlohmann::json& shader_file) {
  render::RasterizerDescr rasterizer{};

  if (!shader_file.contains(kCometEditorShaderKeyRasterizer)) {
    rasterizer.is_wireframe = false;
    rasterizer.is_depth_bias = false;
    rasterizer.cull_mode = render::CullMode::Back;
    return rasterizer;
  }

  const auto& raw_rasterizer{shader_file[kCometEditorShaderKeyRasterizer]};

  if (!raw_rasterizer.is_object()) {
    COMET_LOG_ERROR(LoggerType::External,
                    "shader_export_utils::GetRasterizerDescr",
                    "rasterizer value is not an object", "type",
                    raw_rasterizer.type_name());
    rasterizer.is_wireframe = false;
    rasterizer.is_depth_bias = false;
    rasterizer.cull_mode = render::CullMode::Back;
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

render::CompareOp GetCompareOp(std::string_view raw_compare_op) {
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpNever) {
    return render::CompareOp::Never;
  }
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpLess) {
    return render::CompareOp::Less;
  }
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpEqual) {
    return render::CompareOp::Equal;
  }
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpLessOrEqual) {
    return render::CompareOp::LessOrEqual;
  }
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpGreater) {
    return render::CompareOp::Greater;
  }
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpNotEqual) {
    return render::CompareOp::NotEqual;
  }
  if (raw_compare_op ==
      kCometEditorShaderKeyDepthStencilCompareOpGreaterOrEqual) {
    return render::CompareOp::GreaterOrEqual;
  }
  if (raw_compare_op == kCometEditorShaderKeyDepthStencilCompareOpAlways) {
    return render::CompareOp::Always;
  }

  COMET_LOG_ERROR(LoggerType::External, "shader_export_utils::GetCompareOp",
                  "compare op is unsupported", "compare_op", raw_compare_op);
  return render::CompareOp::Less;
}

render::DepthStencilDescr GetDepthStencilDescr(
    const nlohmann::json& shader_file) {
  render::DepthStencilDescr depth_stencil{};

  if (!shader_file.contains(kCometEditorShaderKeyDepthStencil)) {
    depth_stencil.is_depth_test = true;
    depth_stencil.is_depth_write = true;
    depth_stencil.compare_op = render::CompareOp::Less;
    return depth_stencil;
  }

  const auto& raw_depth_stencil{shader_file[kCometEditorShaderKeyDepthStencil]};

  if (!raw_depth_stencil.is_object()) {
    COMET_LOG_ERROR(LoggerType::External,
                    "shader_export_utils::GetDepthStencilDescr",
                    "depth stencil value is not an object", "type",
                    raw_depth_stencil.type_name());

    depth_stencil.is_depth_test = true;
    depth_stencil.is_depth_write = true;
    depth_stencil.compare_op = render::CompareOp::Less;
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

render::PrimitiveTopology GetPrimitiveTopology(
    std::string_view raw_topology) {
  if (raw_topology == kCometEditorShaderKeyTopologyPoints) {
    return render::PrimitiveTopology::Points;
  }

  if (raw_topology == kCometEditorShaderKeyTopologyLines) {
    return render::PrimitiveTopology::Lines;
  }

  if (raw_topology == kCometEditorShaderKeyTopologyLineStrip) {
    return render::PrimitiveTopology::LineStrip;
  }

  if (raw_topology == kCometEditorShaderKeyTopologyTriangles) {
    return render::PrimitiveTopology::Triangles;
  }

  if (raw_topology == kCometEditorShaderKeyTopologyTriangleStrip) {
    return render::PrimitiveTopology::TriangleStrip;
  }

  COMET_LOG_ERROR(
      LoggerType::External, "shader_export_utils::GetPrimitiveTopology",
      "primitive topology is unsupported", "topology", raw_topology);

  return render::PrimitiveTopology::Unknown;
}

render::ShaderVertexLayout GetShaderVertexLayout(
    std::string_view raw_vertex_layout) {
  if (raw_vertex_layout == kCometEditorShaderKeyVertexLayoutNone) {
    return render::ShaderVertexLayout::None;
  }

  if (raw_vertex_layout == kCometEditorShaderKeyVertexLayoutSkinnedVertex) {
    return render::ShaderVertexLayout::SkinnedVertex;
  }

  if (raw_vertex_layout == kCometEditorShaderKeyVertexLayoutDebugLine) {
    return render::ShaderVertexLayout::DebugLine;
  }

  COMET_LOG_ERROR(LoggerType::External,
                  "shader_export_utils::GetShaderVertexLayout",
                  "shader vertex layout is unsupported", "vertex_layout",
                  raw_vertex_layout);

  return render::ShaderVertexLayout::None;
}

render::ShaderVariableType GetShaderVariableType(
    std::string_view raw_data_type) {
  if (raw_data_type == kCometEditorShaderKeyVariableTypeB32) {
    return render::ShaderVariableType::B32;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeS32) {
    return render::ShaderVariableType::S32;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeU32) {
    return render::ShaderVariableType::U32;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeF32) {
    return render::ShaderVariableType::F32;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeF64) {
    return render::ShaderVariableType::F64;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeB32Vec2) {
    return render::ShaderVariableType::B32Vec2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeB32Vec3) {
    return render::ShaderVariableType::B32Vec3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeB32Vec4) {
    return render::ShaderVariableType::B32Vec4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeS32Vec2) {
    return render::ShaderVariableType::S32Vec2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeS32Vec3) {
    return render::ShaderVariableType::S32Vec3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeS32Vec4) {
    return render::ShaderVariableType::S32Vec4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeU32Vec2) {
    return render::ShaderVariableType::U32Vec2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeU32Vec3) {
    return render::ShaderVariableType::U32Vec3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeU32Vec4) {
    return render::ShaderVariableType::U32Vec4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeVec2) {
    return render::ShaderVariableType::Vec2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeVec3) {
    return render::ShaderVariableType::Vec3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeVec4) {
    return render::ShaderVariableType::Vec4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeSVec2) {
    return render::ShaderVariableType::S32Vec2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeSVec3) {
    return render::ShaderVariableType::S32Vec3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeSVec4) {
    return render::ShaderVariableType::S32Vec4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeF64Vec2) {
    return render::ShaderVariableType::F64Vec2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeF64Vec3) {
    return render::ShaderVariableType::F64Vec3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeF64Vec4) {
    return render::ShaderVariableType::F64Vec4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat2 ||
      raw_data_type == kCometEditorShaderKeyVariableTypeMat2x2) {
    return render::ShaderVariableType::Mat2x2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat2x3) {
    return render::ShaderVariableType::Mat2x3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat2x4) {
    return render::ShaderVariableType::Mat2x4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat3x2) {
    return render::ShaderVariableType::Mat3x2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat3 ||
      raw_data_type == kCometEditorShaderKeyVariableTypeMat3x3) {
    return render::ShaderVariableType::Mat3x3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat3x4) {
    return render::ShaderVariableType::Mat3x4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat4x2) {
    return render::ShaderVariableType::Mat4x2;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat4x3) {
    return render::ShaderVariableType::Mat4x3;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeMat4 ||
      raw_data_type == kCometEditorShaderKeyVariableTypeMat4x4) {
    return render::ShaderVariableType::Mat4x4;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeSampler) {
    return render::ShaderVariableType::Sampler;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeImage) {
    return render::ShaderVariableType::Image;
  }

  if (raw_data_type == kCometEditorShaderKeyVariableTypeAtomic) {
    return render::ShaderVariableType::Atomic;
  }

  COMET_LOG_ERROR(
      LoggerType::External, "shader_export_utils::GetShaderVariableType",
      "shader variable type is unsupported", "variable_type", raw_data_type);

  return render::ShaderVariableType::Unknown;
}

render::ShaderBindingType GetShaderBindingType(
    std::string_view raw_binding_type) {
  if (raw_binding_type == kCometEditorShaderKeyBindingTypeUniformBuffer) {
    return render::ShaderBindingType::UniformBuffer;
  }

  if (raw_binding_type == kCometEditorShaderKeyBindingTypeStorageBuffer) {
    return render::ShaderBindingType::StorageBuffer;
  }

  if (raw_binding_type ==
      kCometEditorShaderKeyBindingTypeCombinedImageSampler) {
    return render::ShaderBindingType::CombinedImageSampler;
  }

  if (raw_binding_type == kCometEditorShaderKeyBindingTypeSampledImage) {
    return render::ShaderBindingType::SampledImage;
  }

  if (raw_binding_type == kCometEditorShaderKeyBindingTypeSampler) {
    return render::ShaderBindingType::Sampler;
  }

  if (raw_binding_type == kCometEditorShaderKeyBindingTypeStorageImage) {
    return render::ShaderBindingType::StorageImage;
  }

  COMET_LOG_ERROR(
      LoggerType::External, "shader_export_utils::GetShaderBindingType",
      "shader binding type is unsupported", "binding_type", raw_binding_type);

  return render::ShaderBindingType::Unknown;
}

render::ShaderBindingScope GetShaderBindingScope(
    std::string_view raw_binding_scope) {
  if (raw_binding_scope == kCometEditorShaderKeyBindingScopeGlobal) {
    return render::ShaderBindingScope::Global;
  }

  if (raw_binding_scope == kCometEditorShaderKeyBindingScopeMaterial) {
    return render::ShaderBindingScope::Material;
  }

  if (raw_binding_scope == kCometEditorShaderKeyBindingScopePass) {
    return render::ShaderBindingScope::Pass;
  }

  if (raw_binding_scope == kCometEditorShaderKeyBindingScopeDraw) {
    return render::ShaderBindingScope::Draw;
  }

  COMET_LOG_ERROR(LoggerType::External,
                  "shader_export_utils::GetShaderBindingScope",
                  "shader binding scope is unsupported", "binding_scope",
                  raw_binding_scope);

  return render::ShaderBindingScope::Unknown;
}

render::ShaderMemoryLayout GetShaderMemoryLayout(
    std::string_view raw_layout) {
  if (raw_layout == kCometEditorShaderKeyMemoryLayoutStd140) {
    return render::ShaderMemoryLayout::Std140;
  }

  if (raw_layout == kCometEditorShaderKeyMemoryLayoutStd430) {
    return render::ShaderMemoryLayout::Std430;
  }

  if (raw_layout == kCometEditorShaderKeyMemoryLayoutPacked) {
    return render::ShaderMemoryLayout::Packed;
  }

  COMET_LOG_ERROR(
      LoggerType::External, "shader_export_utils::GetShaderMemoryLayout",
      "shader memory layout is unsupported", "memory_layout", raw_layout);

  return render::ShaderMemoryLayout::Unknown;
}

render::ShaderStageFlags GetShaderStageFlags(
    const nlohmann::json& raw_stages, memory::Allocator* allocator) {
  render::ShaderStageFlags stages{render::kShaderStageFlagBitsNone};

  if (!raw_stages.is_array()) {
    COMET_LOG_ERROR(
        LoggerType::External, "shader_export_utils::GetShaderStageFlags",
        "stages value is not an array", "type", raw_stages.type_name());
    return stages;
  }

  const auto raw_stages_list{
      nlohmann::json_to_array<std::string>(raw_stages, allocator)};

  for (const auto& raw_stage : raw_stages_list) {
    if (raw_stage == kCometEditorShaderKeyStageCompute) {
      stages |= render::kShaderStageFlagBitsCompute;
    } else if (raw_stage == kCometEditorShaderKeyStageVertex) {
      stages |= render::kShaderStageFlagBitsVertex;
    } else if (raw_stage == kCometEditorShaderKeyStageFragment) {
      stages |= render::kShaderStageFlagBitsFragment;
    } else {
      COMET_LOG_ERROR(LoggerType::External,
                      "shader_export_utils::GetShaderStageFlags",
                      "shader stage is unsupported", "stage", raw_stage);
    }
  }

  return stages;
}

render::ShaderImageBindingSemantic GetShaderImageBindingSemantic(
    std::string_view raw_semantic) {
  if (raw_semantic == kCometEditorShaderKeyImageSemanticMaterialDiffuse) {
    return render::ShaderImageBindingSemantic::MaterialDiffuse;
  }

  if (raw_semantic == kCometEditorShaderKeyImageSemanticMaterialSpecular) {
    return render::ShaderImageBindingSemantic::MaterialSpecular;
  }

  if (raw_semantic == kCometEditorShaderKeyImageSemanticMaterialNormal) {
    return render::ShaderImageBindingSemantic::MaterialNormal;
  }

  if (raw_semantic == kCometEditorShaderKeyImageSemanticMaterialTextures) {
    return render::ShaderImageBindingSemantic::MaterialTextures;
  }

  if (raw_semantic == kCometEditorShaderKeyImageSemanticMainShadowMap) {
    return render::ShaderImageBindingSemantic::MainShadowMap;
  }

  if (raw_semantic == kCometEditorShaderKeyImageSemanticShadowMaps) {
    return render::ShaderImageBindingSemantic::ShadowMaps;
  }

  COMET_LOG_ERROR(LoggerType::External,
                  "shader_export_utils::GetShaderImageBindingSemantic",
                  "shader image binding semantic is unsupported", "semantic",
                  raw_semantic);

  return render::ShaderImageBindingSemantic::Unknown;
}
}  // namespace assetc
}  // namespace tool
}  // namespace comet