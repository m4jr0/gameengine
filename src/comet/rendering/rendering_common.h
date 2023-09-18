// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_COMMON_H_
#define COMET_COMET_RENDERING_RENDERING_COMMON_H_

#include "comet_precompile.h"

#include "comet/math/bounding_volume.h"
#include "comet/math/vector.h"
#include "comet/rendering/camera/frustum.h"

namespace comet {
namespace rendering {
constexpr auto kMaxAppNameLen{256};
constexpr auto kMaxWindowNameLen{256};

constexpr math::Vec3 kColorBlack{0.0f, 0.0f, 0.0f};
constexpr math::Vec3 kColorWhite{1.0f, 1.0f, 1.0f};
constexpr math::Vec3 kColorRed{1.0f, 0.0f, 0.0f};
constexpr math::Vec3 kColorGreen{0.0f, 1.0f, 0.0f};
constexpr math::Vec3 kColorBlue{0.0f, 0.0f, 1.0f};
constexpr math::Vec3 kColorYellow{1.0f, 1.0f, 0.0f};
constexpr math::Vec3 kColorCyan{0.0f, 1.0f, 1.0f};
constexpr math::Vec3 kColorMagenta{1.0f, 0.0f, 1.0f};

enum class DriverType : u8 { Unknown = 0, Empty, OpenGl, Vulkan, Direct3d12 };

DriverType GetDriverTypeFromStr(std::string_view str);
const schar* GetDriverTypeLabel(rendering::DriverType type);

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

struct Vertex {
  math::Vec3 position{};
  math::Vec3 normal{};
  math::Vec3 tangent{};
  math::Vec3 bitangent{};
  math::Vec2 uv{};
  math::Vec3 color{};
};

using Index = u32;

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
  f32 clear_color[4]{kColorBlack[0], kColorBlack[1], kColorBlack[2], 1.0f};
  RenderingViewId id{kInvalidRenderingViewId};
};

enum class ShaderModuleType : u8 { Unknown = 0, Vertex, Fragment };

using ShaderVertexAttributeSize = u32;
constexpr auto kInvalidShaderVertexAttributeSize{
    static_cast<ShaderVertexAttributeSize>(-1)};

enum class ShaderVertexAttributeType : u8 {
  Unknown = 0,
  F16,
  F32,
  F64,
  Vec2,
  Vec3,
  Vec4,
  S8,
  S16,
  S32,
  U8,
  U16,
  U32
};

enum class ShaderUniformType : u8 {
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

using Alignment = uindex;
constexpr auto kInvalidAlignment{static_cast<Alignment>(-1)};

Alignment GetScalarAlignment(ShaderUniformType type);
Alignment GetStd140Alignment(ShaderUniformType type);
Alignment GetStd430Alignment(ShaderUniformType type);

constexpr auto kVertexAttributeDescrMaxNameLen{64};

struct ShaderVertexAttributeDescr {
  ShaderVertexAttributeType type{ShaderVertexAttributeType::Unknown};
  schar name[kVertexAttributeDescrMaxNameLen]{'\0'};
  uindex name_len{0};
};

void SetName(ShaderVertexAttributeDescr& descr, const schar* name,
             uindex name_len);

using ShaderUniformSize = u32;
constexpr auto kInvalidShaderUniformSize{static_cast<ShaderUniformSize>(-1)};

enum class ShaderUniformScope : u8 { Unknown = 0, Global, Instance, Local };
constexpr auto kShaderUniformDescrMaxNameLen{64};

struct ShaderUniformDescr {
  ShaderUniformType type{ShaderUniformType::Unknown};
  ShaderUniformScope scope{ShaderUniformScope::Unknown};
  schar name[kShaderUniformDescrMaxNameLen]{'\0'};
  uindex name_len{0};
};

void SetName(ShaderUniformDescr& descr, const schar* name, uindex name_len);

constexpr auto kMaxShaderCount{256};
constexpr auto kMaxShaderUniformCount{128};
constexpr auto kMaxShaderTextureMapCount{32};

enum class CullMode { Unknown = 0, None, Front, Back, FrontAndBack };

void GenerateGeometry(const math::Aabb& aabb, std::vector<Vertex>& vertices,
                      std::vector<Index>& indices, bool is_visible);

void GenerateGeometry(const Frustum& frustum, std::vector<Vertex>& vertices,
                      std::vector<Index>& indices);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_COMMON_H_
