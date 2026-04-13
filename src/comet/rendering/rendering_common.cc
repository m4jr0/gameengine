// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "rendering_common.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/core/logger.h"
#include "comet/math/plane.h"

namespace comet {
namespace rendering {
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
  [[maybe_unused]] bool is_intersection{false};

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
}  // namespace rendering
}  // namespace comet