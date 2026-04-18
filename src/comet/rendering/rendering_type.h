// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_TYPE_
#define COMET_COMET_RENDERING_RENDERING_TYPE_

// External. ///////////////////////////////////////////////////////////////////
#include <string_view>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/string_id.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"

namespace comet {
namespace rendering {
constexpr auto kMaxAppNameLen{256};
constexpr auto kMaxWindowNameLen{256};

constexpr math::Vec3 kColorBlackRgb{.0f, .0f, .0f};
constexpr math::Vec3 kColorWhiteRgb{1.0f, 1.0f, 1.0f};
constexpr math::Vec3 kColorRedRgb{1.0f, .0f, .0f};
constexpr math::Vec3 kColorGreenRgb{.0f, 1.0f, .0f};
constexpr math::Vec3 kColorBlueRgb{.0f, .0f, 1.0f};
constexpr math::Vec3 kColorYellowRgb{1.0f, 1.0f, .0f};
constexpr math::Vec3 kColorCyanRgb{.0f, 1.0f, 1.0f};
constexpr math::Vec3 kColorMagentaRgb{1.0f, .0f, 1.0f};

constexpr math::Vec4 kColorBlackRgba{kColorBlackRgb, 1.0f};
constexpr math::Vec4 kColorWhiteRgba{kColorWhiteRgb, 1.0f};
constexpr math::Vec4 kColorRedRgba{kColorRedRgb, 1.0f};
constexpr math::Vec4 kColorGreenRgba{kColorGreenRgb, 1.0f};
constexpr math::Vec4 kColorBlueRgba{kColorBlueRgb, 1.0f};
constexpr math::Vec4 kColorYellowRgba{kColorYellowRgb, 1.0f};
constexpr math::Vec4 kColorCyanRgba{kColorCyanRgb, 1.0f};
constexpr math::Vec4 kColorMagentaRgba{kColorMagentaRgb, 1.0f};

enum class DriverType : u8 { Unknown = 0, Empty, OpenGl, Vulkan, Direct3d12 };

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

enum class TextureFormat : u32 { Unknown = 0, Rgba8, Rgb8 };

enum class RenderingViewType : u16 {
  Unknown = 0,
  World,
  Shadow,
  Skybox,
  Debug,
  ImGui
};

enum class RenderingViewMatrixSource : u8 {
  Unknown = 0,
  SceneCamera,
  UiCamera
};

using RenderingViewId = stringid::StringId;
constexpr auto kInvalidRenderingViewId{static_cast<RenderingViewId>(-1)};

struct RenderingViewDescr {
  RenderingViewMatrixSource matrix_source{RenderingViewMatrixSource::Unknown};
  RenderingViewType type{RenderingViewType::Unknown};
  WindowSize width{0};
  WindowSize height{0};
  f32 clear_color[4]{kColorBlackRgb[0], kColorBlackRgb[1], kColorBlackRgb[2],
                     1.0f};
  RenderingViewId id{kInvalidRenderingViewId};
};

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

const schar* GetShaderVertexLayoutLabel(ShaderVertexLayout layout);

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

enum class CullMode { Unknown = 0, None, Front, Back, FrontAndBack };

struct RasterizerDescr {
  bool is_wireframe{false};
  bool is_depth_bias{false};
  CullMode cull_mode{CullMode::Unknown};
};

enum class CompareOp : u8 {
  Unknown = 0,
  Never,
  Less,
  Equal,
  LessOrEqual,
  Greater,
  NotEqual,
  GreaterOrEqual,
  Always
};

struct DepthStencilDescr {
  bool is_depth_test{true};
  bool is_depth_write{true};
  CompareOp compare_op{CompareOp::Less};
};

enum class PrimitiveTopology {
  Unknown = 0,
  Points,
  Lines,
  LineStrip,
  Triangles,
  TriangleStrip
};

struct RenderCameraData {
  math::Mat4 projection_matrix{1.0f};
  math::Mat4 view_matrix{1.0f};
  math::Vec3 view_position{.0f};
  f32 near_plane{.05f};
  math::Vec3 front{.0f, .0f, -1.0f};
  f32 far_plane{1000.0f};
  math::Vec3 up{.0f, 1.0f, .0f};
  f32 fov_y_radians{.0f};
  math::Vec3 right{1.0f, .0f, .0f};
  f32 aspect_ratio{1.0f};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_TYPE_