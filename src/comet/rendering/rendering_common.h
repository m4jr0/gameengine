// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_COMMON_H_
#define COMET_COMET_RENDERING_RENDERING_COMMON_H_

#include <string_view>

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/geometry/geometry_common.h"
#include "comet/math/bounding_volume.h"
#include "comet/math/vector.h"
#include "comet/rendering/camera/frustum.h"

namespace comet {
namespace rendering {
constexpr auto kMaxAppNameLen{256};
constexpr auto kMaxWindowNameLen{256};

constexpr math::Vec3 kColorBlackRgb{0.0f, 0.0f, 0.0f};
constexpr math::Vec3 kColorWhiteRgb{1.0f, 1.0f, 1.0f};
constexpr math::Vec3 kColorRedRgb{1.0f, 0.0f, 0.0f};
constexpr math::Vec3 kColorGreenRgb{0.0f, 1.0f, 0.0f};
constexpr math::Vec3 kColorBlueRgb{0.0f, 0.0f, 1.0f};
constexpr math::Vec3 kColorYellowRgb{1.0f, 1.0f, 0.0f};
constexpr math::Vec3 kColorCyanRgb{0.0f, 1.0f, 1.0f};
constexpr math::Vec3 kColorMagentaRgb{1.0f, 0.0f, 1.0f};

constexpr math::Vec4 kColorBlackRgba{kColorBlackRgb, 1.0f};
constexpr math::Vec4 kColorWhiteRgba{kColorWhiteRgb, 1.0f};
constexpr math::Vec4 kColorRedRgba{kColorRedRgb, 1.0f};
constexpr math::Vec4 kColorGreenRgba{kColorGreenRgb, 1.0f};
constexpr math::Vec4 kColorBlueRgba{kColorBlueRgb, 1.0f};
constexpr math::Vec4 kColorYellowRgba{kColorYellowRgb, 1.0f};
constexpr math::Vec4 kColorCyanRgba{kColorCyanRgb, 1.0f};
constexpr math::Vec4 kColorMagentaRgba{kColorMagentaRgb, 1.0f};

enum class DriverType : u8 { Unknown = 0, Empty, OpenGl, Vulkan, Direct3d12 };

DriverType GetDriverTypeFromStr(std::string_view str);
const schar* GetDriverTypeLabel(DriverType type);
bool IsMultithreading([[maybe_unused]] DriverType type);

using FrameCount = u32;
constexpr auto kInvalidFrameCount{static_cast<FrameCount>(-1)};

enum class AntiAliasingType : u16 {
  None = 0,
  Msaa,
  MsaaX2,
  MsaaX4,
  MsaaX8,
  MsaaX16,
  MsaaX32,
  MsaaX64
};

AntiAliasingType GetAntiAliasingTypeFromStr(std::string_view str);

using WindowSize = u16;

enum class TextureType : u8 {
  Unknown = 0,
  Ambient,
  Diffuse,
  Specular,
  Normal,
  Color
};

enum class TextureRepeatMode : u8 {
  Unknown = 0,
  Repeat,
  MirroredRepeat,
  ClampToEdge,
  ClampToBorder
};

enum class TextureFilterMode : u8 { Unknown = 0, Nearest, Linear };

const schar* GetTextureTypeLabel(TextureType texture_type);
const schar* GetTextureFilterModeLabel(TextureFilterMode filter_mode);
const schar* GetTextureRepeatModeLabel(TextureRepeatMode repeat_mode);

enum class TextureFormat : u32 { Unknown = 0, Rgba8, Rgb8 };

enum class RenderingViewType : u16 {
  Unknown = 0,
  World,
  Skybox,
  SimpleWorld,
  Debug,
  ImGui
};

enum class RenderingViewMatrixSource : u8 {
  Unknown = 0,
  SceneCamera,
  UiCamera,
  LightCamera
};

using RenderingViewId = stringid::StringId;
constexpr auto kInvalidRenderingViewId{static_cast<RenderingViewId>(-1)};

struct RenderingViewDescr {
  bool is_first{false};
  bool is_last{false};
  RenderingViewMatrixSource matrix_source{RenderingViewMatrixSource::Unknown};
  RenderingViewType type{RenderingViewType::Unknown};
  WindowSize width{0};
  WindowSize height{0};
  f32 clear_color[4]{kColorBlackRgb[0], kColorBlackRgb[1], kColorBlackRgb[2],
                     1.0f};
  RenderingViewId id{kInvalidRenderingViewId};
};

static constexpr auto kShaderLocalSize{256};

enum class ShaderModuleType : u8 { Unknown = 0, Compute, Vertex, Fragment };

constexpr auto kMaxShaderDefineNameLen{31};
constexpr auto kMaxShaderDefineValueLen{31};

struct ShaderDefineDescr {
  schar name[kMaxShaderDefineNameLen + 1]{'\0'};
  schar value[kMaxShaderDefineValueLen + 1]{'\0'};
  usize name_len{0};
  usize value_len{0};
};

using ShaderVertexAttributeSize = u32;
constexpr auto kInvalidShaderVertexAttributeSize{
    static_cast<ShaderVertexAttributeSize>(-1)};

enum class ShaderVertexAttributeType : u8 {
  Unknown = 0,
  S8,
  S16,
  S32,
  U8,
  U16,
  U32,
  F16,
  F32,
  F64,
  U8Vec2,
  U8Vec3,
  U8Vec4,
  U16Vec2,
  U16Vec3,
  U16Vec4,
  U32Vec2,
  U32Vec3,
  U32Vec4,
  S8Vec2,
  S8Vec3,
  S8Vec4,
  S16Vec2,
  S16Vec3,
  S16Vec4,
  S32Vec2,
  S32Vec3,
  S32Vec4,
  F16Vec2,
  F16Vec3,
  F16Vec4,
  F32Vec2,
  F32Vec3,
  F32Vec4,
  F64Vec2,
  F64Vec3,
  F64Vec4
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

Alignment GetScalarAlignment(ShaderVariableType type);
Alignment GetStd140Alignment(ShaderVariableType type);
Alignment GetStd430Alignment(ShaderVariableType type);

constexpr auto kVertexAttributeDescrMaxNameLen{63};

struct ShaderVertexAttributeDescr {
  ShaderVertexAttributeType type{ShaderVertexAttributeType::Unknown};
  schar name[kVertexAttributeDescrMaxNameLen + 1]{'\0'};
  usize name_len{0};
};

void SetName(ShaderVertexAttributeDescr& descr, const schar* name,
             usize name_len);

enum ShaderStageFlagBits {
  kShaderStageFlagBitsNone = 0x0,
  kShaderStageFlagBitsCompute = 0x1,
  kShaderStageFlagBitsVertex = 0x2,
  kShaderStageFlagBitsFragment = 0x4
};

using ShaderStageFlags = u8;

using ShaderUniformSize = u32;
constexpr auto kInvalidShaderUniformSize{static_cast<ShaderUniformSize>(-1)};

enum class ShaderUniformScope : u8 { Unknown = 0, Global, Instance };
constexpr auto kShaderUniformDescrMaxNameLen{63};

struct ShaderUniformDescr {
  ShaderVariableType type{ShaderVariableType::Unknown};
  ShaderUniformScope scope{ShaderUniformScope::Unknown};
  ShaderStageFlags stages{kShaderStageFlagBitsNone};
  schar name[kShaderUniformDescrMaxNameLen + 1]{'\0'};
  usize name_len{0};
};

using ShaderConstantSize = u32;
constexpr auto kInvalidShaderConstantSize{static_cast<ShaderConstantSize>(-1)};

constexpr auto kShaderConstantDescrMaxNameLen{63};

struct ShaderConstantDescr {
  ShaderVariableType type{ShaderVariableType::Unknown};
  ShaderStageFlags stages{kShaderStageFlagBitsNone};
  schar name[kShaderConstantDescrMaxNameLen + 1]{'\0'};
  usize name_len{0};
};

constexpr auto kShaderStoragePropertyDescrMaxNameLen{63};

struct ShaderStoragePropertyDescr {
  ShaderVariableType type{ShaderVariableType::Unknown};
  schar name[kShaderStoragePropertyDescrMaxNameLen + 1]{'\0'};
  usize name_len{0};
};

constexpr auto kShaderStorageDescrMaxNameLen{63};

struct ShaderStorageDescr {
  schar name[kShaderStorageDescrMaxNameLen + 1]{'\0'};
  schar engine_define[kMaxShaderDefineNameLen + 1]{'\0'};
  ShaderStageFlags stages{kShaderStageFlagBitsNone};
  usize name_len{0};
  usize engine_define_len{0};
  Array<ShaderStoragePropertyDescr> properties{};
};

void SetName(ShaderUniformDescr& descr, const schar* name, usize name_len);
void SetName(ShaderConstantDescr& descr, const schar* name, usize name_len);
void SetName(ShaderStorageDescr& descr, const schar* name, usize name_len);
void SetEngineDefine(ShaderStorageDescr& descr, const schar* engine_define,
                     usize engine_define_len);
void SetName(ShaderStoragePropertyDescr& descr, const schar* name,
             usize name_len);
void SetName(ShaderDefineDescr& descr, const schar* name, usize name_len);
void SetValue(ShaderDefineDescr& descr, const schar* value, usize value_len);

constexpr auto kMaxShaderCount{256};
constexpr auto kMaxShaderUniformCount{128};
constexpr auto kMaxShaderConstantCount{32};
constexpr auto kMaxShaderTextureMapCount{32};

enum class CullMode { Unknown = 0, None, Front, Back, FrontAndBack };

enum class PrimitiveTopology {
  Unknown = 0,
  Points,
  Lines,
  LineStrip,
  Triangles,
  TriangleStrip
};

void GenerateGeometry(const math::Aabb& aabb,
                      Array<geometry::SkinnedVertex>& vertices,
                      Array<geometry::Index>& indices, bool is_visible);

void GenerateGeometry(const Frustum& frustum,
                      Array<geometry::SkinnedVertex>& vertices,
                      Array<geometry::Index>& indices);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_COMMON_H_
