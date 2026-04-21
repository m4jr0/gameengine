// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_TYPE_RENDERING_SHADER_TYPE_H_
#define COMET_COMET_RENDERING_TYPE_RENDERING_SHADER_TYPE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"

namespace comet {
namespace rendering {
static constexpr auto kShaderLocalSize{256};

enum class ShaderStage : u8 { Unknown = 0, Compute, Vertex, Fragment };

constexpr auto kMaxShaderDefineNameLen{31};
constexpr auto kMaxShaderDefineValueLen{31};

struct ShaderDefineDescr {
  schar name[kMaxShaderDefineNameLen + 1]{'\0'};
  schar value[kMaxShaderDefineValueLen + 1]{'\0'};
  usize name_len{0};
  usize value_len{0};
};

enum class ShaderVariableType : u8 {
  Unknown = 0,
  B32,
  S32,
  U32,
  F32,
  F64,
  B32Vec2,
  B32Vec3,
  B32Vec4,
  S32Vec2,
  S32Vec3,
  S32Vec4,
  U32Vec2,
  U32Vec3,
  U32Vec4,
  Vec2,
  Vec3,
  Vec4,
  F64Vec2,
  F64Vec3,
  F64Vec4,
  Mat2x2,
  Mat2x3,
  Mat2x4,
  Mat3x2,
  Mat3x3,
  Mat3x4,
  Mat4x2,
  Mat4x3,
  Mat4x4,
  Sampler,
  Image,
  Atomic
};

using Alignment = usize;
constexpr auto kInvalidAlignment{static_cast<Alignment>(-1)};

using ShaderVariableSize = u32;
constexpr auto kInvalidShaderVariableSize{static_cast<ShaderVariableSize>(-1)};

enum ShaderStageFlagBits {
  kShaderStageFlagBitsNone = 0x0,
  kShaderStageFlagBitsCompute = 0x1,
  kShaderStageFlagBitsVertex = 0x2,
  kShaderStageFlagBitsFragment = 0x4
};

using ShaderStageFlags = u8;

enum class ShaderBindingType : u8 {
  Unknown = 0,
  UniformBuffer,
  StorageBuffer,
  CombinedImageSampler,
  SampledImage,
  Sampler,
  StorageImage
};

enum class ShaderBindingScope : u8 {
  Unknown = 0,
  Global,
  Material,
  Pass,
  Draw
};

enum class ShaderMemoryLayout : u8 { Unknown = 0, Std140, Std430, Packed };

constexpr auto kShaderNameMaxLen{63};

struct ShaderNamedDescr {
  schar name[kShaderNameMaxLen + 1]{'\0'};
  usize name_len{0};
};

enum class ShaderVertexLayout : u8 {
  None = 0,

  // Static meshes currently bind to the skinned layout too.
  // Joint indices / weights are simply unused by static shaders.
  SkinnedVertex,
  DebugLine
};

struct ShaderFieldDescr : ShaderNamedDescr {
  ShaderVariableType type{ShaderVariableType::Unknown};
  u32 array_count{1};
};

enum class ShaderImageBindingSemantic : u8 {
  Unknown = 0,

  // Material.
  MaterialDiffuse,
  MaterialSpecular,
  MaterialNormal,
  MaterialTextures,

  // Global / shadowing.
  MainShadowMap,
  ShadowMaps,

  // Future deferred / generic.
  GbufferAlbedo,
  GbufferNormal,
  GbufferDepth,
};

struct ShaderBindingDescr : ShaderNamedDescr {
  ShaderBindingType type{ShaderBindingType::Unknown};
  ShaderBindingScope scope{ShaderBindingScope::Unknown};
  ShaderMemoryLayout layout{ShaderMemoryLayout::Unknown};
  ShaderStageFlags stages{kShaderStageFlagBitsNone};
  u32 set{0};
  u32 binding{0};
  u32 descriptor_count{1};
  ShaderImageBindingSemantic image_semantic{
      ShaderImageBindingSemantic::Unknown};
  Array<ShaderFieldDescr> fields{};
};

struct ShaderPushConstantDescr : ShaderNamedDescr {
  ShaderStageFlags stages{kShaderStageFlagBitsNone};
  Array<ShaderFieldDescr> fields{};
};

constexpr auto kMaxShaderCount{256};
constexpr auto kMaxShaderTextureMapCount{32};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_TYPE_RENDERING_SHADER_TYPE_H_