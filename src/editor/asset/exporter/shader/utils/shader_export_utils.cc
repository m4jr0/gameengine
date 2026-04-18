// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "shader_export_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "editor/asset/asset_utils.h"
#include "editor/asset/exporter/shader/data/shader_export_keys.h"

namespace comet {
namespace editor {
namespace asset {
rendering::CullMode GetCullMode(std::string_view raw_cull_mode) {
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

rendering::RasterizerDescr GetRasterizerDescr(
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

rendering::CompareOp GetCompareOp(std::string_view raw_compare_op) {
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

rendering::DepthStencilDescr GetDepthStencilDescr(
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

rendering::PrimitiveTopology GetPrimitiveTopology(
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

rendering::ShaderVertexLayout GetShaderVertexLayout(
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

rendering::ShaderVariableType GetShaderVariableType(
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

rendering::ShaderBindingType GetShaderBindingType(
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

rendering::ShaderBindingScope GetShaderBindingScope(
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

rendering::ShaderMemoryLayout GetShaderMemoryLayout(
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

rendering::ShaderStageFlags GetShaderStageFlags(
    const nlohmann::json& raw_stages, memory::Allocator* allocator) {
  rendering::ShaderStageFlags stages{rendering::kShaderStageFlagBitsNone};

  if (!raw_stages.is_array()) {
    COMET_LOG_GLOBAL_ERROR("Wrong type found for stages array: ",
                           raw_stages.type_name(), "! Ignoring.");
    return stages;
  }

  const auto raw_stages_list{
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

rendering::ShaderImageBindingSemantic GetShaderImageBindingSemantic(
    std::string_view raw_semantic) {
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
}  // namespace asset
}  // namespace editor
}  // namespace comet