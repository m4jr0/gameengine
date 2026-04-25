// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "shader_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace rendering {
const schar* GetShaderStageLabel(ShaderStage stage) {
  switch (stage) {
    case ShaderStage::Unknown:
      return "unknown";
    case ShaderStage::Compute:
      return "compute";
    case ShaderStage::Vertex:
      return "vertex";
    case ShaderStage::Fragment:
      return "fragment";
    default:
      return kUnknownLabel;
  }
}

const schar* GetShaderVariableTypeLabel(ShaderVariableType type) {
  switch (type) {
    case ShaderVariableType::Unknown:
      return "unknown";
    case ShaderVariableType::B32:
      return "b32";
    case ShaderVariableType::S32:
      return "s32";
    case ShaderVariableType::U32:
      return "u32";
    case ShaderVariableType::F32:
      return "f32";
    case ShaderVariableType::F64:
      return "f64";
    case ShaderVariableType::B32Vec2:
      return "b32_vec2";
    case ShaderVariableType::B32Vec3:
      return "b32_vec3";
    case ShaderVariableType::B32Vec4:
      return "b32_vec4";
    case ShaderVariableType::S32Vec2:
      return "s32_vec2";
    case ShaderVariableType::S32Vec3:
      return "s32_vec3";
    case ShaderVariableType::S32Vec4:
      return "s32_vec4";
    case ShaderVariableType::U32Vec2:
      return "u32_vec2";
    case ShaderVariableType::U32Vec3:
      return "u32_vec3";
    case ShaderVariableType::U32Vec4:
      return "u32_vec4";
    case ShaderVariableType::Vec2:
      return "vec2";
    case ShaderVariableType::Vec3:
      return "vec3";
    case ShaderVariableType::Vec4:
      return "vec4";
    case ShaderVariableType::F64Vec2:
      return "f64_vec2";
    case ShaderVariableType::F64Vec3:
      return "f64_vec3";
    case ShaderVariableType::F64Vec4:
      return "f64_vec4";
    case ShaderVariableType::Mat2x2:
      return "mat2x2";
    case ShaderVariableType::Mat2x3:
      return "mat2x3";
    case ShaderVariableType::Mat2x4:
      return "mat2x4";
    case ShaderVariableType::Mat3x2:
      return "mat3x2";
    case ShaderVariableType::Mat3x3:
      return "mat3x3";
    case ShaderVariableType::Mat3x4:
      return "mat3x4";
    case ShaderVariableType::Mat4x2:
      return "mat4x2";
    case ShaderVariableType::Mat4x3:
      return "mat4x3";
    case ShaderVariableType::Mat4x4:
      return "mat4x4";
    case ShaderVariableType::Sampler:
      return "sampler";
    case ShaderVariableType::Image:
      return "image";
    case ShaderVariableType::Atomic:
      return "atomic";
    default:
      return kUnknownLabel;
  }
}

const schar* GetShaderBindingTypeLabel(ShaderBindingType type) {
  switch (type) {
    case ShaderBindingType::Unknown:
      return "unknown";
    case ShaderBindingType::UniformBuffer:
      return "uniform_buffer";
    case ShaderBindingType::StorageBuffer:
      return "storage_buffer";
    case ShaderBindingType::CombinedImageSampler:
      return "combined_image_sampler";
    case ShaderBindingType::SampledImage:
      return "sampled_image";
    case ShaderBindingType::Sampler:
      return "sampler";
    case ShaderBindingType::StorageImage:
      return "storage_image";
    default:
      return kUnknownLabel;
  }
}

const schar* GetShaderBindingScopeLabel(ShaderBindingScope scope) {
  switch (scope) {
    case ShaderBindingScope::Unknown:
      return "unknown";
    case ShaderBindingScope::Global:
      return "global";
    case ShaderBindingScope::Material:
      return "material";
    case ShaderBindingScope::Pass:
      return "pass";
    case ShaderBindingScope::Draw:
      return "draw";
    default:
      return kUnknownLabel;
  }
}

const schar* GetShaderMemoryLayoutLabel(ShaderMemoryLayout layout) {
  switch (layout) {
    case ShaderMemoryLayout::Unknown:
      return "unknown";
    case ShaderMemoryLayout::Std140:
      return "std140";
    case ShaderMemoryLayout::Std430:
      return "std430";
    case ShaderMemoryLayout::Packed:
      return "packed";
    default:
      return kUnknownLabel;
  }
}

const schar* GetShaderVertexLayoutLabel(ShaderVertexLayout layout) {
  switch (layout) {
    case ShaderVertexLayout::None:
      return "none";
    case ShaderVertexLayout::SkinnedVertex:
      return "skinned_vertex";
    case ShaderVertexLayout::DebugLine:
      return "debug_line";
    default:
      return kUnknownLabel;
  }
}

const schar* GetShaderImageBindingSemanticLabel(
    ShaderImageBindingSemantic semantic) {
  switch (semantic) {
    case ShaderImageBindingSemantic::Unknown:
      return "unknown";
    case ShaderImageBindingSemantic::MaterialDiffuse:
      return "material_diffuse";
    case ShaderImageBindingSemantic::MaterialSpecular:
      return "material_specular";
    case ShaderImageBindingSemantic::MaterialNormal:
      return "material_normal";
    case ShaderImageBindingSemantic::MaterialTextures:
      return "material_textures";
    case ShaderImageBindingSemantic::MainShadowMap:
      return "main_shadow_map";
    case ShaderImageBindingSemantic::ShadowMaps:
      return "shadow_maps";
    case ShaderImageBindingSemantic::GbufferAlbedo:
      return "gbuffer_albedo";
    case ShaderImageBindingSemantic::GbufferNormal:
      return "gbuffer_normal";
    case ShaderImageBindingSemantic::GbufferDepth:
      return "gbuffer_depth";
    default:
      return kUnknownLabel;
  }
}
}  // namespace rendering
}  // namespace comet