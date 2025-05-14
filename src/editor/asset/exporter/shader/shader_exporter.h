// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_EXPORTER_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/type/tstring.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/shader_resource.h"
#include "editor/asset/exporter/asset_exporter.h"

using namespace std::literals;

namespace comet {
namespace editor {
namespace asset {
static constexpr auto kCometEditorShaderKeyIsWireframe{"is_wireframe"sv};
static constexpr auto kCometEditorShaderKeyCullMode{"cull_mode"sv};
static constexpr auto kCometEditorShaderKeyTopology{"topology"sv};
static constexpr auto kCometEditorShaderKeyShaderModulePaths{
    "shader_module_paths"sv};
static constexpr auto kCometEditorShaderKeyDefines{"defines"sv};
static constexpr auto kCometEditorShaderKeyVertexAttributes{
    "vertex_attributes"sv};
static constexpr auto kCometEditorShaderKeyVertexAttributeName{"name"sv};
static constexpr auto kCometEditorShaderKeyVertexAttributeType{"type"sv};

static constexpr auto kCometEditorShaderKeyUniforms{"uniforms"sv};
static constexpr auto kCometEditorShaderKeyUniformName{"name"sv};
static constexpr auto kCometEditorShaderKeyUniformType{"type"sv};
static constexpr auto kCometEditorShaderKeyUniformScope{"scope"sv};
static constexpr auto kCometEditorShaderKeyUniformStages{"stages"sv};

static constexpr auto kCometEditorShaderKeyUniformScopeGlobal{"global"sv};
static constexpr auto kCometEditorShaderKeyUniformScopeInstance{"instance"sv};

static constexpr auto kCometEditorShaderKeyConstants{"constants"sv};
static constexpr auto kCometEditorShaderKeyConstantName{"name"sv};
static constexpr auto kCometEditorShaderKeyConstantType{"type"sv};
static constexpr auto kCometEditorShaderKeyConstantStages{"stages"sv};

static constexpr auto kCometEditorShaderKeyStorages{"storages"sv};
static constexpr auto kCometEditorShaderKeyStorageName{"name"sv};
static constexpr auto kCometEditorShaderKeyStorageStages{"stages"sv};
static constexpr auto kCometEditorShaderKeyStorageLayout{"layout"sv};
static constexpr auto kCometEditorShaderKeyStorageEngineDefine{
    "engine_define"sv};
static constexpr auto kCometEditorShaderKeyStoragePropertyName{"name"sv};
static constexpr auto kCometEditorShaderKeyStoragePropertyType{"type"sv};

static constexpr auto kCometEditorShaderKeyCullModeNone{"none"sv};
static constexpr auto kCometEditorShaderKeyCullModeFront{"front"sv};
static constexpr auto kCometEditorShaderKeyCullModeBack{"back"sv};
static constexpr auto kCometEditorShaderKeyCullModeFrontAndBack{
    "front_and_back"sv};

static constexpr auto kCometEditorShaderKeyTopologyPoints{"points"sv};
static constexpr auto kCometEditorShaderKeyTopologyLines{"lines"sv};
static constexpr auto kCometEditorShaderKeyTopologyLineStrip{"line_strip"sv};
static constexpr auto kCometEditorShaderKeyTopologyTriangles{"triangles"sv};
static constexpr auto kCometEditorShaderKeyTopologyTriangleStrip{
    "triangle_strip"sv};

static constexpr auto kCometEditorShaderKeyAttributeTypeS8{"s8"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeS16{"s16"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeS32{"s32"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeU8{"u8"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeU16{"u16"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeU32{"u32"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeF16{"f16"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeF32{"f32"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeF64{"f64"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeU8Vec4{"u8vec4"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeS8Vec4{"s8vec4"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeU16Vec2{"u16vec2"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeU16Vec3{"u16vec3"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeU16Vec4{"u16vec4"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeS16Vec2{"s16vec2"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeS16Vec3{"s16vec3"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeS16Vec4{"s16vec4"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeF16Vec2{"f16vec2"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeF16Vec4{"f16vec4"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeU32Vec2{"u32vec2"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeU32Vec3{"u32vec3"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeU32Vec4{"u32vec4"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeS32Vec2{"s32vec2"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeS32Vec3{"s32vec3"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeS32Vec4{"s32vec4"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeF32Vec2{"f32vec2"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeF32Vec3{"f32vec3"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeF32Vec4{"f32vec4"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeF64Vec2{"f64vec2"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeF64Vec3{"f64vec3"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeF64Vec4{"f64vec4"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeVec2{"vec2"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeVec3{"vec3"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeVec4{"vec4"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeUVec2{"uvec2"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeUVec3{"uvec3"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeUVec4{"uvec4"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeSVec2{"svec2"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeSVec3{"svec3"sv};
static constexpr auto kCometEditorShaderKeyAttributeTypeSVec4{"svec4"sv};

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

static constexpr auto kCometEditorShaderKeyDefineName{"name"sv};
static constexpr auto kCometEditorShaderKeyDefineValue{"value"sv};

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
  void PopulateFiles(ResourceFilesContext& context) const override;

 private:
  struct ShaderContext {
    schar* file{nullptr};
    usize file_len{0};
    usize file_buffer_len{0};
    const tchar* asset_abs_path{nullptr};
  };

  static void DumpShaderModules(const nlohmann::json& shader_file,
                                resource::ShaderResource& shader);
  static void DumpDefines(const nlohmann::json& shader_file,
                          memory::Allocator* allocator,
                          resource::ShaderResource& shader);
  static void DumpVertexAttributes(const nlohmann::json& shader_file,
                                   resource::ShaderResource& shader);
  static void DumpUniforms(const nlohmann::json& shader_file,
                           memory::Allocator* allocator,
                           resource::ShaderResource& shader);
  static void DumpConstants(const nlohmann::json& shader_file,
                            memory::Allocator* allocator,
                            resource::ShaderResource& shader);
  static void DumpStorages(const nlohmann::json& shader_file,
                           memory::Allocator* allocator,
                           resource::ShaderResource& shader);

  static rendering::CullMode GetCullMode(std::string_view raw_cull_mode);
  static rendering::PrimitiveTopology GetPrimitiveTopology(
      std::string_view raw_topology);
  static rendering::ShaderVertexAttributeType GetShaderVertexAttributeType(
      std::string_view raw_vertex_attribute_type);
  static rendering::ShaderVariableType GetShaderVariableType(
      std::string_view raw_uniform_type);
  static rendering::ShaderUniformScope GetShaderUniformScope(
      std::string_view raw_uniform_scope);
  static rendering::ShaderStageFlags GetShaderStageFlags(
      const nlohmann::json& raw_stages, memory::Allocator* allocator);
  static void OnShaderSizeRequest(job::IOJobParamsHandle params_handle);
  static void OnShaderLoading(job::IOJobParamsHandle params_handle);
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_EXPORTER_H_
