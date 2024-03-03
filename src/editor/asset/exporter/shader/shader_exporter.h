// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_EXPORTER_H_

#include "comet_precompile.h"

#include "comet/core/type/tstring.h"
#include "comet/rendering/rendering_common.h"
#include "editor/asset/exporter/asset_exporter.h"

using namespace std::literals;

namespace comet {
namespace editor {
namespace asset {
static constexpr auto kCometEditorShaderKeyIsWireframe{"is_wireframe"sv};
static constexpr auto kCometEditorShaderKeyCullMode{"cull_mode"sv};
static constexpr auto kCometEditorShaderKeyShaderModulePaths{
    "shader_module_paths"sv};
static constexpr auto kCometEditorShaderKeyVertexAttributes{
    "vertex_attributes"sv};
static constexpr auto kCometEditorShaderKeyVertexAttributeName{"name"sv};
static constexpr auto kCometEditorShaderKeyVertexAttributeType{"type"sv};

static constexpr auto kCometEditorShaderKeyUniforms{"uniforms"sv};
static constexpr auto kCometEditorShaderKeyUniformName{"name"sv};
static constexpr auto kCometEditorShaderKeyUniformType{"type"sv};
static constexpr auto kCometEditorShaderKeyUniformScope{"scope"sv};

static constexpr auto kCometEditorShaderCullModeNone{"none"sv};
static constexpr auto kCometEditorShaderCullModeFront{"front"sv};
static constexpr auto kCometEditorShaderCullModeBack{"back"sv};
static constexpr auto kCometEditorShaderCullModeFrontAndBack{
    "front_and_back"sv};

static constexpr auto kCometEditorShaderAttributeTypeF16{"f16"sv};
static constexpr auto kCometEditorShaderAttributeTypeF32{"f32"sv};
static constexpr auto kCometEditorShaderAttributeTypeF64{"f64"sv};
static constexpr auto kCometEditorShaderAttributeTypeVec2{"vec2"sv};
static constexpr auto kCometEditorShaderAttributeTypeVec3{"vec3"sv};
static constexpr auto kCometEditorShaderAttributeTypeVec4{"vec4"sv};
static constexpr auto kCometEditorShaderAttributeTypeS8{"s8"sv};
static constexpr auto kCometEditorShaderAttributeTypeS16{"s16"sv};
static constexpr auto kCometEditorShaderAttributeTypeS32{"s32"sv};
static constexpr auto kCometEditorShaderAttributeTypeU8{"u8"sv};
static constexpr auto kCometEditorShaderAttributeTypeU16{"u16"sv};
static constexpr auto kCometEditorShaderAttributeTypeU32{"u32"sv};

static constexpr auto kCometEditorShaderUniformTypeB32{"b32"sv};
static constexpr auto kCometEditorShaderUniformTypeS32{"s32"sv};
static constexpr auto kCometEditorShaderUniformTypeU32{"u32"sv};
static constexpr auto kCometEditorShaderUniformTypeF32{"f32"sv};
static constexpr auto kCometEditorShaderUniformTypeF64{"f64"sv};
static constexpr auto kCometEditorShaderUniformTypeB32Vec2{"b32vec2"sv};
static constexpr auto kCometEditorShaderUniformTypeB32Vec3{"b32vec3"sv};
static constexpr auto kCometEditorShaderUniformTypeB32Vec4{"b32vec4"sv};
static constexpr auto kCometEditorShaderUniformTypeS32Vec2{"s32vec2"sv};
static constexpr auto kCometEditorShaderUniformTypeS32Vec3{"s32vec3"sv};
static constexpr auto kCometEditorShaderUniformTypeS32Vec4{"s32vec4"sv};
static constexpr auto kCometEditorShaderUniformTypeU32Vec2{"u32vec2"sv};
static constexpr auto kCometEditorShaderUniformTypeU32Vec3{"u32vec3"sv};
static constexpr auto kCometEditorShaderUniformTypeU32Vec4{"u32vec4"sv};
static constexpr auto kCometEditorShaderUniformTypeVec2{"vec2"sv};
static constexpr auto kCometEditorShaderUniformTypeVec3{"vec3"sv};
static constexpr auto kCometEditorShaderUniformTypeVec4{"vec4"sv};
static constexpr auto kCometEditorShaderUniformTypeF64Vec2{"f64vec2"sv};
static constexpr auto kCometEditorShaderUniformTypeF64Vec3{"f64vec3"sv};
static constexpr auto kCometEditorShaderUniformTypeF64Vec4{"f64vec4"sv};
static constexpr auto kCometEditorShaderUniformTypeMat2x2{"mat2x2"sv};
static constexpr auto kCometEditorShaderUniformTypeMat2x3{"mat2x3"sv};
static constexpr auto kCometEditorShaderUniformTypeMat2x4{"mat2x4"sv};
static constexpr auto kCometEditorShaderUniformTypeMat2{"mat2"sv};
static constexpr auto kCometEditorShaderUniformTypeMat3x2{"mat3x2"sv};
static constexpr auto kCometEditorShaderUniformTypeMat3x3{"mat3x3"sv};
static constexpr auto kCometEditorShaderUniformTypeMat3x4{"mat3x4"sv};
static constexpr auto kCometEditorShaderUniformTypeMat3{"mat3"sv};
static constexpr auto kCometEditorShaderUniformTypeMat4x2{"mat4x2"sv};
static constexpr auto kCometEditorShaderUniformTypeMat4x3{"mat4x3"sv};
static constexpr auto kCometEditorShaderUniformTypeMat4x4{"mat4x4"sv};
static constexpr auto kCometEditorShaderUniformTypeMat4{"mat4"sv};
static constexpr auto kCometEditorShaderUniformTypeSampler{"sampler"sv};
static constexpr auto kCometEditorShaderUniformTypeImage{"image"sv};
static constexpr auto kCometEditorShaderUniformTypeAtomic{"atomic"sv};

static constexpr auto kCometEditorShaderUniformScopeGlobal{"global"sv};
static constexpr auto kCometEditorShaderUniformScopeInstance{"instance"sv};
static constexpr auto kCometEditorShaderUniformScopeLocal{"local"sv};

class ShaderExporter : public AssetExporter {
 public:
  ShaderExporter() = default;
  ShaderExporter(const ShaderExporter&) = delete;
  ShaderExporter(ShaderExporter&&) = delete;
  ShaderExporter& operator=(const ShaderExporter&) = delete;
  ShaderExporter& operator=(ShaderExporter&&) = delete;
  virtual ~ShaderExporter() = default;

  bool IsCompatible(CTStringView extension) const override;

 protected:
  std::vector<resource::ResourceFile> GetResourceFiles(
      AssetDescr& asset_descr) const override;

 private:
  static rendering::CullMode GetCullMode(std::string_view raw_cull_mode);
  static rendering::ShaderVertexAttributeType GetShaderVertexAttributeType(
      std::string_view raw_vertex_attribute_type);
  static rendering::ShaderUniformType GetShaderUniformType(
      std::string_view raw_uniform_type);
  static rendering::ShaderUniformScope GetShaderUniformScope(
      std::string_view raw_uniform_scope);
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_EXPORTER_H_
