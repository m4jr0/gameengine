// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "rendering_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/core/logger.h"
#include "comet/math/geometry.h"
#include "comet/math/math_scalar.h"
#include "comet/math/numeric_utils.h"
#include "comet/math/plane.h"

namespace comet {
namespace rendering {
namespace internal {
math::Vec3 GetFallbackTangentAxis(const math::Vec3& normal) {
  return math::Abs(normal.y) < .999f ? math::Vec3{.0f, 1.0f, .0f}
                                     : math::Vec3{1.0f, .0f, .0f};
}

math::Vec3 SanitizeNormal(math::Vec3 normal) {
  if (!math::IsAlmostZero(math::GetSquaredMagnitude(normal))) {
    math::Normalize(normal);
    return normal;
  }

  return math::Vec3{.0f, 1.0f, .0f};
}
}  // namespace internal

math::Vec3 ComputeStableUpVector(const math::Vec3& forward) {
  math::Vec3 dir{math::GetNormalizedCopy(forward)};
  math::Vec3 world_up{.0f, 1.0f, .0f};
  math::Vec3 fallback_up{.0f, .0f, 1.0f};

  if (math::Abs(math::Dot(dir, world_up)) > math::kParallelThreshold) {
    return fallback_up;
  }

  return world_up;
}

math::Mat4 LookAt(const math::Vec3& eye, const math::Vec3& target,
                  const math::Vec3& world_up) {
  auto forward{target - eye};
  math::Normalize(forward);

  auto right{math::Cross(forward, world_up)};
  math::Normalize(right);

  auto up{math::Cross(right, forward)};
  math::Normalize(up);

  // Convert from world coordinates to view coordinates.
  // Following matrix is a product of a translation and rotation matrices, which
  // themselves are the transpose of their respective matrices to convert from
  // view coordinates to world coordinates.
  math::Mat4 matrix{.0f};
  matrix[0][0] = right.x;
  matrix[1][0] = right.y;
  matrix[2][0] = right.z;

  matrix[0][1] = up.x;
  matrix[1][1] = up.y;
  matrix[2][1] = up.z;

  matrix[0][2] = -forward.x;
  matrix[1][2] = -forward.y;
  matrix[2][2] = -forward.z;

  matrix[3][0] = -math::Dot(right, eye);
  matrix[3][1] = -math::Dot(up, eye);
  matrix[3][2] = math::Dot(forward, eye);
  matrix[3][3] = 1.0f;

  return matrix;
}

math::Mat4 GeneratePerspectiveMatrix(f32 vertical_fov, f32 ratio, f32 z_near,
                                     f32 z_far,
                                     ClipSpaceDepthRange depth_range) {
  const f32 tan_half_fov{math::Tan(vertical_fov * .5f)};
  math::Mat4 matrix{.0f};

  matrix[0][0] = 1.0f / (ratio * tan_half_fov);
  matrix[1][1] = 1.0f / tan_half_fov;
  matrix[2][3] = -1.0f;

  if (depth_range == ClipSpaceDepthRange::MinusOneToOne) {
    matrix[2][2] = -(z_far + z_near) / (z_far - z_near);
    matrix[3][2] = -(2.0f * z_far * z_near) / (z_far - z_near);
  } else {
    matrix[2][2] = -z_far / (z_far - z_near);
    matrix[3][2] = -(z_far * z_near) / (z_far - z_near);
  }

  return matrix;
}

math::Mat4 GenerateOrthographicMatrix(f32 left, f32 right, f32 bottom, f32 top,
                                      f32 z_near, f32 z_far,
                                      ClipSpaceDepthRange depth_range) {
  math::Mat4 matrix{.0f};

  matrix[0][0] = 2.0f / (right - left);
  matrix[1][1] = 2.0f / (top - bottom);
  matrix[3][0] = -(right + left) / (right - left);
  matrix[3][1] = -(top + bottom) / (top - bottom);
  matrix[3][3] = 1.0f;

  if (depth_range == ClipSpaceDepthRange::MinusOneToOne) {
    matrix[2][2] = -2.0f / (z_far - z_near);
    matrix[3][2] = -(z_far + z_near) / (z_far - z_near);
  } else {
    matrix[2][2] = -1.0f / (z_far - z_near);
    matrix[3][2] = -z_near / (z_far - z_near);
  }

  return matrix;
}

math::Vec4 GenerateTangentWithSign(const math::Vec3& raw_normal,
                                   const math::Vec3& raw_tangent,
                                   const math::Vec3* raw_bitangent) {
  const auto normal{internal::SanitizeNormal(raw_normal)};
  auto tangent{raw_tangent};

  if (math::IsAlmostZero(math::GetSquaredMagnitude(tangent))) {
    tangent = math::Cross(internal::GetFallbackTangentAxis(normal), normal);
  }

  tangent = tangent - normal * math::Dot(normal, tangent);

  if (math::IsAlmostZero(math::GetSquaredMagnitude(tangent))) {
    tangent = math::Cross(internal::GetFallbackTangentAxis(normal), normal);
  }

  math::Normalize(tangent);
  auto sign{1.0f};

  if (raw_bitangent != nullptr) {
    auto bitangent{*raw_bitangent};

    if (!math::IsAlmostZero(math::GetSquaredMagnitude(bitangent))) {
      math::Normalize(bitangent);
      sign = math::Dot(math::Cross(normal, tangent), bitangent) < .0f ? -1.0f
                                                                      : 1.0f;
    }
  }

  return math::Vec4{tangent.x, tangent.y, tangent.z, sign};
}

DriverType GetDriverTypeFromStr(std::string_view str) {
  if (str == conf::kRenderingDriverOpengl) {
    return DriverType::OpenGl;
  } else if (str == conf::kRenderingDriverVulkan) {
    return DriverType::Vulkan;
  } else if (str == conf::kRenderingDriverDirect3d12) {
    return DriverType::Direct3d12;
  }
#ifdef COMET_DEBUG
  else if (str == conf::kRenderingDriverEmpty) {
    return DriverType::Empty;
  }
#endif  // COMET_DEBUG

  return DriverType::Unknown;
}

const schar* GetDriverTypeLabel(DriverType type) {
  switch (type) {
    case DriverType::OpenGl:
      return "OpenGL";
    case DriverType::Vulkan:
      return "Vulkan";
    case DriverType::Direct3d12:
      return "Direct3D 12";
#ifdef COMET_DEBUG
    case DriverType::Empty:
      return "Empty";
#endif  // COMET_DEBUG
    default:
      return "???";
  }
}

DriverType GetDriverType() {
  return GetDriverTypeFromStr(COMET_CONF_STR(conf::kRenderingDriver));
}

bool IsMultithreading([[maybe_unused]] DriverType type) {
#ifdef COMET_ENABLE_RENDERDOC_COMPATIBILITY
#ifndef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  COMET_ASSERT(false,
               "To enable RenderDoc compatibility, the main thread worker must "
               "be allowed to remain disabled.");
#endif  // !COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  return false;
#else
  switch (type) {
    case DriverType::OpenGl:
      return false;
    case DriverType::Vulkan:
      return true;
    case DriverType::Direct3d12:
      return true;
#ifdef COMET_DEBUG
    case DriverType::Empty:
      return true;
#endif  // COMET_DEBUG
    default:
      return false;
  }
#endif  // COMET_ENABLE_RENDERDOC_COMPATIBILITY
}

AntiAliasingType GetAntiAliasingTypeFromStr(std::string_view str) {
  if (str == conf::kRenderingAntiAliasingTypeNone) {
    return AntiAliasingType::None;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaaX64) {
    return AntiAliasingType::MsaaX64;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaaX32) {
    return AntiAliasingType::MsaaX32;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaaX16) {
    return AntiAliasingType::MsaaX16;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaaX8) {
    return AntiAliasingType::MsaaX8;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaaX4) {
    return AntiAliasingType::MsaaX4;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaaX2) {
    return AntiAliasingType::MsaaX2;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaa) {
    return AntiAliasingType::Msaa;
  }

  return AntiAliasingType::None;
}

const schar* GetTextureTypeLabel(TextureType texture_type) {
  switch (texture_type) {
    case TextureType::Unknown:
      return "unknown";
    case TextureType::Ambient:
      return "ambient";
    case TextureType::Diffuse:
      return "diffuse";
    case TextureType::Specular:
      return "specular";
    case TextureType::Normal:
      return "normal";
    case TextureType::Color:
      return "color";
  }

  return "???";
}

const schar* GetTextureFilterModeLabel(TextureFilterMode filter_mode) {
  switch (filter_mode) {
    case TextureFilterMode::Unknown:
      return "unknown";
    case TextureFilterMode::Linear:
      return "linear";
    case TextureFilterMode::Nearest:
      return "nearest";
  }

  return "???";
}

const schar* GetTextureRepeatModeLabel(TextureRepeatMode repeat_mode) {
  switch (repeat_mode) {
    case TextureRepeatMode::Unknown:
      return "unknown";
    case TextureRepeatMode::Repeat:
      return "repeat";
    case TextureRepeatMode::MirroredRepeat:
      return "mirrored repeat";
    case TextureRepeatMode::ClampToEdge:
      return "clamp to edge";
    case TextureRepeatMode::ClampToBorder:
      return "clamp to border";
  }

  return "???";
}

Alignment GetScalarAlignment(ShaderVariableType type) {
  switch (type) {
    case ShaderVariableType::B32:
    case ShaderVariableType::S32:
    case ShaderVariableType::U32:
    case ShaderVariableType::F32:
      return 4;

    case ShaderVariableType::F64:
      return 8;

    case ShaderVariableType::B32Vec2:
    case ShaderVariableType::B32Vec3:
    case ShaderVariableType::B32Vec4:
    case ShaderVariableType::S32Vec2:
    case ShaderVariableType::S32Vec3:
    case ShaderVariableType::S32Vec4:
    case ShaderVariableType::U32Vec2:
    case ShaderVariableType::U32Vec3:
    case ShaderVariableType::U32Vec4:
    case ShaderVariableType::Vec2:
    case ShaderVariableType::Vec3:
    case ShaderVariableType::Vec4:
      return 4;

    case ShaderVariableType::F64Vec2:
    case ShaderVariableType::F64Vec3:
    case ShaderVariableType::F64Vec4:
      return 8;

    case ShaderVariableType::Mat2x2:
    case ShaderVariableType::Mat2x3:
    case ShaderVariableType::Mat2x4:
    case ShaderVariableType::Mat3x2:
    case ShaderVariableType::Mat3x3:
    case ShaderVariableType::Mat3x4:
    case ShaderVariableType::Mat4x2:
    case ShaderVariableType::Mat4x3:
    case ShaderVariableType::Mat4x4:
      return 4;

    default:
      return kInvalidAlignment;
  }
}

Alignment GetStd140Alignment(ShaderVariableType type) {
  switch (type) {
    case ShaderVariableType::B32:
    case ShaderVariableType::S32:
    case ShaderVariableType::U32:
    case ShaderVariableType::F32:
      return 4;

    case ShaderVariableType::F64:
      return 8;

    case ShaderVariableType::B32Vec2:
    case ShaderVariableType::S32Vec2:
    case ShaderVariableType::U32Vec2:
    case ShaderVariableType::Vec2:
      return 8;

    case ShaderVariableType::B32Vec3:
    case ShaderVariableType::B32Vec4:
    case ShaderVariableType::S32Vec3:
    case ShaderVariableType::S32Vec4:
    case ShaderVariableType::U32Vec3:
    case ShaderVariableType::U32Vec4:
    case ShaderVariableType::Vec3:
    case ShaderVariableType::Vec4:
      return 16;

    case ShaderVariableType::F64Vec2:
      return 16;

    case ShaderVariableType::F64Vec3:
    case ShaderVariableType::F64Vec4:
      return 32;

    case ShaderVariableType::Mat2x2:
    case ShaderVariableType::Mat2x3:
    case ShaderVariableType::Mat2x4:
    case ShaderVariableType::Mat3x2:
    case ShaderVariableType::Mat3x3:
    case ShaderVariableType::Mat3x4:
    case ShaderVariableType::Mat4x2:
    case ShaderVariableType::Mat4x3:
    case ShaderVariableType::Mat4x4:
      return 16;

    default:
      return kInvalidAlignment;
  }
}

Alignment GetStd430Alignment(ShaderVariableType type) {
  switch (type) {
    case ShaderVariableType::B32:
    case ShaderVariableType::S32:
    case ShaderVariableType::U32:
    case ShaderVariableType::F32:
      return 4;

    case ShaderVariableType::F64:
      return 8;

    case ShaderVariableType::B32Vec2:
    case ShaderVariableType::S32Vec2:
    case ShaderVariableType::U32Vec2:
    case ShaderVariableType::Vec2:
      return 8;

    case ShaderVariableType::B32Vec3:
    case ShaderVariableType::B32Vec4:
    case ShaderVariableType::S32Vec3:
    case ShaderVariableType::S32Vec4:
    case ShaderVariableType::U32Vec3:
    case ShaderVariableType::U32Vec4:
    case ShaderVariableType::Vec3:
    case ShaderVariableType::Vec4:
      return 16;

    case ShaderVariableType::F64Vec2:
      return 16;

    case ShaderVariableType::F64Vec3:
    case ShaderVariableType::F64Vec4:
      return 32;

    case ShaderVariableType::Mat2x2:
    case ShaderVariableType::Mat2x3:
    case ShaderVariableType::Mat2x4:
      return 8;

    case ShaderVariableType::Mat3x2:
    case ShaderVariableType::Mat3x3:
    case ShaderVariableType::Mat3x4:
    case ShaderVariableType::Mat4x2:
    case ShaderVariableType::Mat4x3:
    case ShaderVariableType::Mat4x4:
      return 16;

    default:
      return kInvalidAlignment;
  }
}

ShaderVariableSize GetShaderVariableTypeSize(ShaderVariableType type) {
  switch (type) {
    case ShaderVariableType::B32:
    case ShaderVariableType::S32:
    case ShaderVariableType::U32:
    case ShaderVariableType::F32:
      return 4;

    case ShaderVariableType::B32Vec2:
    case ShaderVariableType::S32Vec2:
    case ShaderVariableType::U32Vec2:
    case ShaderVariableType::Vec2:
      return 2 * 4;

    case ShaderVariableType::B32Vec3:
    case ShaderVariableType::S32Vec3:
    case ShaderVariableType::U32Vec3:
    case ShaderVariableType::Vec3:
      return 3 * 4;

    case ShaderVariableType::B32Vec4:
    case ShaderVariableType::S32Vec4:
    case ShaderVariableType::U32Vec4:
    case ShaderVariableType::Vec4:
      return 4 * 4;

    case ShaderVariableType::F64:
      return 8;

    case ShaderVariableType::F64Vec2:
      return 2 * 8;

    case ShaderVariableType::F64Vec3:
      return 3 * 8;

    case ShaderVariableType::F64Vec4:
      return 4 * 8;

    case ShaderVariableType::Mat2x2:
      return 2 * 2 * 4;

    case ShaderVariableType::Mat2x3:
    case ShaderVariableType::Mat3x2:
      return 2 * 3 * 4;

    case ShaderVariableType::Mat3x3:
      return 3 * 3 * 4;

    case ShaderVariableType::Mat2x4:
    case ShaderVariableType::Mat4x2:
      return 2 * 4 * 4;

    case ShaderVariableType::Mat3x4:
    case ShaderVariableType::Mat4x3:
      return 3 * 4 * 4;

    case ShaderVariableType::Mat4x4:
      return 4 * 4 * 4;

    case ShaderVariableType::Sampler:
    case ShaderVariableType::Image:
    case ShaderVariableType::Atomic:
    case ShaderVariableType::Unknown:
      return kInvalidShaderVariableSize;
  }

  return kInvalidShaderVariableSize;
}

const schar* GetShaderVertexLayoutLabel(ShaderVertexLayout layout) {
  switch (layout) {
    case ShaderVertexLayout::None:
      return "none";
    case ShaderVertexLayout::SkinnedVertex:
      return "skinned vertex";
    case ShaderVertexLayout::DebugLine:
      return "debug_line";
    default:
      return "???";
  }
}

namespace internal {
template <typename TDescr>
void SetShaderNameInternal(TDescr& descr, const schar* name, usize name_len) {
  descr.name_len = name_len;

  if (descr.name_len >= kShaderNameMaxLen) {
    COMET_LOG_RENDERING_WARNING(
        "Shader name provided is too long: ", descr.name_len,
        " >= ", kShaderNameMaxLen, ". It will be truncated.");
    descr.name_len = static_cast<usize>(kShaderNameMaxLen);
  }

  Copy(descr.name, name, descr.name_len);
  descr.name[descr.name_len] = '\0';
}
}  // namespace internal

void SetName(ShaderNamedDescr& descr, const schar* name, usize name_len) {
  internal::SetShaderNameInternal(descr, name, name_len);
}

void SetName(ShaderFieldDescr& descr, const schar* name, usize name_len) {
  internal::SetShaderNameInternal(descr, name, name_len);
}

void SetName(ShaderBindingDescr& descr, const schar* name, usize name_len) {
  internal::SetShaderNameInternal(descr, name, name_len);
}

void SetName(ShaderPushConstantDescr& descr, const schar* name,
             usize name_len) {
  internal::SetShaderNameInternal(descr, name, name_len);
}

void SetName(ShaderDefineDescr& descr, const schar* name, usize name_len) {
  descr.name_len = name_len;

  if (descr.name_len >= kMaxShaderDefineNameLen) {
    COMET_LOG_RENDERING_WARNING(
        "Shader define name provided is too long: ", descr.name_len,
        " >= ", kMaxShaderDefineNameLen, ". It will be truncated.");
    descr.name_len = static_cast<usize>(kMaxShaderDefineNameLen);
  }

  Copy(descr.name, name, descr.name_len);
  descr.name[descr.name_len] = '\0';
}

void SetValue(ShaderDefineDescr& descr, const schar* value, usize value_len) {
  descr.value_len = value_len;

  if (descr.value_len >= kMaxShaderDefineValueLen) {
    COMET_LOG_RENDERING_WARNING(
        "Shader define value provided is too long: ", descr.value_len,
        " >= ", kMaxShaderDefineValueLen, ". It will be truncated.");
    descr.value_len = static_cast<usize>(kMaxShaderDefineValueLen);
  }

  Copy(descr.value, value, descr.value_len);
  descr.value[descr.value_len] = '\0';
}

void GenerateGeometry(const math::Aabb& aabb,
                      Array<geometry::SkinnedVertex>& vertices,
                      Array<geometry::Index>& indices, bool is_visible) {
  COMET_ASSERT(vertices.GetSize() == 0,
               "Tried to generate geometry for AABB, but vertices provided are "
               "not empty!");
  COMET_ASSERT(indices.GetSize() == 0,
               "Tried to generate geometry for AABB, but indices provided are "
               "not empty!");

  const math::Vec3 extents{aabb.extents[0], aabb.extents[1], aabb.extents[2]};

  vertices.Reserve(8);

  geometry::SkinnedVertex vertex{};
  vertex.color = math::Vec4{is_visible ? kColorGreenRgb : kColorRedRgb, 1.0f};

  constexpr auto kTopRightFarIndex{0};
  vertex.position = aabb.center + math::Vec3(extents.x, extents.y, -extents.z);
  vertices.PushBack(vertex);

  constexpr auto kTopRightNearIndex{1};
  vertex.position = aabb.center + math::Vec3(extents.x, extents.y, extents.z);
  vertices.PushBack(vertex);

  constexpr auto kTopLeftFarIndex{2};
  vertex.position = aabb.center + math::Vec3(-extents.x, extents.y, -extents.z);
  vertices.PushBack(vertex);

  constexpr auto kTopLeftNearIndex{3};
  vertex.position = aabb.center + math::Vec3(-extents.x, extents.y, extents.z);
  vertices.PushBack(vertex);

  constexpr auto kBottomRightFarIndex{4};
  vertex.position = aabb.center + math::Vec3(extents.x, -extents.y, -extents.z);
  vertices.PushBack(vertex);

  constexpr auto kBottomRightNearIndex{5};
  vertex.position = aabb.center + math::Vec3(extents.x, -extents.y, extents.z);
  vertices.PushBack(vertex);

  constexpr auto kBottomLeftFarIndex{6};
  vertex.position =
      aabb.center + math::Vec3(-extents.x, -extents.y, -extents.z);
  vertices.PushBack(vertex);

  constexpr auto kBottomLeftNearIndex{7};
  vertex.position = aabb.center + math::Vec3(-extents.x, -extents.y, extents.z);
  vertices.PushBack(vertex);

  indices.Reserve(36);
  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kTopRightNearIndex);
  indices.PushBack(kTopLeftNearIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kTopLeftFarIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kTopRightNearIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kBottomRightNearIndex);

  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kBottomLeftFarIndex);

  indices.PushBack(kBottomLeftFarIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kBottomLeftNearIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kBottomLeftNearIndex);
  indices.PushBack(kBottomLeftFarIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kBottomLeftFarIndex);
  indices.PushBack(kTopLeftFarIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kTopLeftFarIndex);

  indices.PushBack(kTopLeftFarIndex);
  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kBottomLeftFarIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kTopRightNearIndex);
  indices.PushBack(kBottomRightNearIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kBottomLeftNearIndex);
}

void GenerateGeometry(const Frustum& frustum,
                      Array<geometry::SkinnedVertex>& vertices,
                      Array<geometry::Index>& indices) {
  COMET_ASSERT(vertices.GetSize() == 0,
               "Tried to generate geometry for frustum, but vertices provided "
               "are not empty!");
  COMET_ASSERT(indices.GetSize() == 0,
               "Tried to generate geometry for frustum, but indices provided "
               "are not empty!");

  const auto& top_face{frustum.GetTop()};
  const auto& bottom_face{frustum.GetBottom()};
  const auto& left_face{frustum.GetLeft()};
  const auto& right_face{frustum.GetRight()};
  const auto& near_face{frustum.GetNear()};
  const auto& far_face{frustum.GetFar()};

  vertices.Reserve(8);

  geometry::SkinnedVertex vertex{};
  vertex.color = math::Vec4{kColorBlackRgb, 1.0f};
  [[maybe_unused]] auto is_intersection{false};

  constexpr auto kTopRightFarIndex{0};
  is_intersection =
      math::Intersect(top_face, far_face, right_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Top, far and right faces in the frustum won't intersect!");
  vertices.PushBack(vertex);

  constexpr auto kTopRightNearIndex{1};
  is_intersection =
      math::Intersect(top_face, near_face, right_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Top, near and right faces in the frustum won't intersect!");
  vertices.PushBack(vertex);

  constexpr auto kTopLeftFarIndex{2};
  is_intersection =
      math::Intersect(top_face, far_face, left_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Top, far and left faces in the frustum won't intersect!");
  vertices.PushBack(vertex);

  constexpr auto kTopLeftNearIndex{3};
  is_intersection =
      math::Intersect(top_face, near_face, left_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Top, near and left faces in the frustum won't intersect!");
  vertices.PushBack(vertex);

  constexpr auto kBottomRightFarIndex{4};
  is_intersection =
      math::Intersect(bottom_face, far_face, right_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Bottom, far and right faces in the frustum won't intersect!");
  vertices.PushBack(vertex);

  constexpr auto kBottomRightNearIndex{5};
  is_intersection =
      math::Intersect(bottom_face, near_face, right_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Bottom, near and right faces in the frustum won't intersect!");
  vertices.PushBack(vertex);

  constexpr auto kBottomLeftFarIndex{6};
  is_intersection =
      math::Intersect(bottom_face, far_face, left_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Bottom, far and left faces in the frustum won't intersect!");
  vertices.PushBack(vertex);

  constexpr auto kBottomLeftNearIndex{7};
  is_intersection =
      math::Intersect(bottom_face, near_face, left_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Bottom, near and left faces in the frustum won't intersect!");
  vertices.PushBack(vertex);

  indices.Reserve(36);
  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kTopRightNearIndex);
  indices.PushBack(kTopLeftNearIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kTopLeftFarIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kTopRightNearIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kBottomRightNearIndex);

  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kBottomLeftFarIndex);

  indices.PushBack(kBottomLeftFarIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kBottomLeftNearIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kBottomLeftNearIndex);
  indices.PushBack(kBottomLeftFarIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kBottomLeftFarIndex);
  indices.PushBack(kTopLeftFarIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kTopLeftFarIndex);

  indices.PushBack(kTopLeftFarIndex);
  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kBottomLeftFarIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kTopRightNearIndex);
  indices.PushBack(kBottomRightNearIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kBottomLeftNearIndex);
}

bool IsBufferBindingType(ShaderBindingType type) {
  return type == ShaderBindingType::UniformBuffer ||
         type == ShaderBindingType::StorageBuffer;
}

bool IsImageBindingType(ShaderBindingType type) {
  return type == ShaderBindingType::CombinedImageSampler ||
         type == ShaderBindingType::SampledImage ||
         type == ShaderBindingType::Sampler ||
         type == ShaderBindingType::StorageImage;
}

bool IsSrgbTextureType(TextureType type) {
  switch (type) {
    case TextureType::Diffuse:
    case TextureType::Color:
      return true;

    case TextureType::Specular:
    case TextureType::Normal:
    case TextureType::Ambient:
    case TextureType::Unknown:
    default:
      return false;
  }
}

u32 GetMipLevels(u32 width, u32 height) {
  return static_cast<u32>(math::Log2(math::Max(width, height))) + 1;
}

u8 GetResolvedChannelCount(TextureFormat format, u8 fallback) {
  switch (format) {
    case TextureFormat::Rgba8:
      return 4;

    case TextureFormat::Rgb8:
      return 3;

    case TextureFormat::Unknown:
    default:
      return fallback;
  }
}
}  // namespace rendering
}  // namespace comet
