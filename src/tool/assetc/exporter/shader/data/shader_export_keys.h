// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_SHADER_DATA_SHADER_EXPORT_KEYS_H_
#define COMET_EDITOR_ASSET_EXPORTER_SHADER_DATA_SHADER_EXPORT_KEYS_H_

#include "comet/core.h"

using namespace std::literals;

namespace comet {
namespace tool {
namespace assetc {
static constexpr auto kCometEditorShaderKeyRasterizer{"rasterizer"sv};
static constexpr auto kCometEditorShaderKeyDepthStencil{"depth_stencil"sv};
static constexpr auto kCometEditorShaderKeyTopology{"topology"sv};
static constexpr auto kCometEditorShaderKeyShaderModulePaths{
    "shader_module_paths"sv};
static constexpr auto kCometEditorShaderKeyDefines{"defines"sv};
static constexpr auto kCometEditorShaderKeyBindings{"bindings"sv};
static constexpr auto kCometEditorShaderKeyPushConstants{"push_constants"sv};

static constexpr auto kCometEditorShaderKeyName{"name"sv};
static constexpr auto kCometEditorShaderKeyType{"type"sv};
static constexpr auto kCometEditorShaderKeyValue{"value"sv};
static constexpr auto kCometEditorShaderKeyScope{"scope"sv};
static constexpr auto kCometEditorShaderKeyLayout{"layout"sv};
static constexpr auto kCometEditorShaderKeyStages{"stages"sv};
static constexpr auto kCometEditorShaderKeySet{"set"sv};
static constexpr auto kCometEditorShaderKeyBinding{"binding"sv};
static constexpr auto kCometEditorShaderKeyDescriptorCount{
    "descriptor_count"sv};
static constexpr auto kCometEditorShaderKeyFields{"fields"sv};
static constexpr auto kCometEditorShaderKeyArrayCount{"array_count"sv};

static constexpr auto kCometEditorShaderKeyRasterizerIsWireframe{
    "is_wireframe"sv};
static constexpr auto kCometEditorShaderKeyRasterizerIsDepthBias{
    "is_depth_bias"sv};
static constexpr auto kCometEditorShaderKeyRasterizerCullMode{"cull_mode"sv};

static constexpr auto kCometEditorShaderKeyRasterizerCullModeNone{"none"sv};
static constexpr auto kCometEditorShaderKeyRasterizerCullModeFront{"front"sv};
static constexpr auto kCometEditorShaderKeyRasterizerCullModeBack{"back"sv};
static constexpr auto kCometEditorShaderKeyRasterizerCullModeFrontAndBack{
    "front_and_back"sv};

static constexpr auto kCometEditorShaderKeyDepthStencilIsDepthTest{
    "is_depth_test"sv};
static constexpr auto kCometEditorShaderKeyDepthStencilIsDepthWrite{
    "is_depth_write"sv};
static constexpr auto kCometEditorShaderKeyDepthStencilCompareOp{
    "compare_op"sv};

static constexpr auto kCometEditorShaderKeyDepthStencilCompareOpNever{
    "never"sv};
static constexpr auto kCometEditorShaderKeyDepthStencilCompareOpLess{"less"sv};
static constexpr auto kCometEditorShaderKeyDepthStencilCompareOpEqual{
    "equal"sv};
static constexpr auto kCometEditorShaderKeyDepthStencilCompareOpLessOrEqual{
    "less_or_equal"sv};
static constexpr auto kCometEditorShaderKeyDepthStencilCompareOpGreater{
    "greater"sv};
static constexpr auto kCometEditorShaderKeyDepthStencilCompareOpNotEqual{
    "not_equal"sv};
static constexpr auto kCometEditorShaderKeyDepthStencilCompareOpGreaterOrEqual{
    "greater_or_equal"sv};
static constexpr auto kCometEditorShaderKeyDepthStencilCompareOpAlways{
    "always"sv};

static constexpr auto kCometEditorShaderKeyTopologyPoints{"points"sv};
static constexpr auto kCometEditorShaderKeyTopologyLines{"lines"sv};
static constexpr auto kCometEditorShaderKeyTopologyLineStrip{"line_strip"sv};
static constexpr auto kCometEditorShaderKeyTopologyTriangles{"triangles"sv};
static constexpr auto kCometEditorShaderKeyTopologyTriangleStrip{
    "triangle_strip"sv};

static constexpr auto kCometEditorShaderKeyVertexLayout{"vertex_layout"sv};
static constexpr auto kCometEditorShaderKeyVertexLayoutNone{"none"sv};
static constexpr auto kCometEditorShaderKeyVertexLayoutSkinnedVertex{
    "skinned_vertex"sv};
static constexpr auto kCometEditorShaderKeyVertexLayoutDebugLine{
    "debug_line"sv};

static constexpr auto kCometEditorShaderKeyVariableTypeB32{"b32"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeS32{"s32"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeU32{"u32"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeF32{"f32"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeF64{"f64"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeB32Vec2{"b32vec2"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeB32Vec3{"b32vec3"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeB32Vec4{"b32vec4"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeS32Vec2{"s32vec2"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeS32Vec3{"s32vec3"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeS32Vec4{"s32vec4"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeU32Vec2{"u32vec2"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeU32Vec3{"u32vec3"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeU32Vec4{"u32vec4"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeVec2{"vec2"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeVec3{"vec3"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeVec4{"vec4"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeSVec2{"svec2"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeSVec3{"svec3"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeSVec4{"svec4"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeF64Vec2{"f64vec2"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeF64Vec3{"f64vec3"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeF64Vec4{"f64vec4"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeMat2x2{"mat2x2"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeMat2x3{"mat2x3"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeMat2x4{"mat2x4"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeMat2{"mat2"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeMat3x2{"mat3x2"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeMat3x3{"mat3x3"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeMat3x4{"mat3x4"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeMat3{"mat3"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeMat4x2{"mat4x2"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeMat4x3{"mat4x3"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeMat4x4{"mat4x4"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeMat4{"mat4"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeSampler{"sampler"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeImage{"image"sv};
static constexpr auto kCometEditorShaderKeyVariableTypeAtomic{"atomic"sv};

static constexpr auto kCometEditorShaderKeyStageCompute{"compute"sv};
static constexpr auto kCometEditorShaderKeyStageVertex{"vertex"sv};
static constexpr auto kCometEditorShaderKeyStageFragment{"fragment"sv};

static constexpr auto kCometEditorShaderKeyBindingTypeUniformBuffer{
    "uniform_buffer"sv};
static constexpr auto kCometEditorShaderKeyBindingTypeStorageBuffer{
    "storage_buffer"sv};
static constexpr auto kCometEditorShaderKeyBindingTypeCombinedImageSampler{
    "combined_image_sampler"sv};
static constexpr auto kCometEditorShaderKeyBindingTypeSampledImage{
    "sampled_image"sv};
static constexpr auto kCometEditorShaderKeyBindingTypeSampler{"sampler"sv};
static constexpr auto kCometEditorShaderKeyBindingTypeStorageImage{
    "storage_image"sv};

static constexpr auto kCometEditorShaderKeyBindingScopeGlobal{"global"sv};
static constexpr auto kCometEditorShaderKeyBindingScopeMaterial{"material"sv};
static constexpr auto kCometEditorShaderKeyBindingScopePass{"pass"sv};
static constexpr auto kCometEditorShaderKeyBindingScopeDraw{"draw"sv};

static constexpr auto kCometEditorShaderKeyMemoryLayoutStd140{"std140"sv};
static constexpr auto kCometEditorShaderKeyMemoryLayoutStd430{"std430"sv};
static constexpr auto kCometEditorShaderKeyMemoryLayoutPacked{"packed"sv};

static constexpr auto kCometEditorShaderKeyBindingImageSemantic{
    "image_semantic"sv};

static constexpr auto kCometEditorShaderKeyImageSemanticMaterialDiffuse{
    "material_diffuse"sv};
static constexpr auto kCometEditorShaderKeyImageSemanticMaterialSpecular{
    "material_specular"sv};
static constexpr auto kCometEditorShaderKeyImageSemanticMaterialNormal{
    "material_normal"sv};
static constexpr auto kCometEditorShaderKeyImageSemanticMaterialTextures{
    "material_textures"sv};
static constexpr auto kCometEditorShaderKeyImageSemanticMainShadowMap{
    "main_shadow_map"sv};
static constexpr auto kCometEditorShaderKeyImageSemanticShadowMaps{
    "shadow_maps"sv};
}  // namespace assetc
}  // namespace tool
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_SHADER_DATA_SHADER_EXPORT_KEYS_H_