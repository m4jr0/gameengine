// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "rendering_common.h"

#include "comet/core/conf/configuration_manager.h"
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
  }

  return "???";
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

Alignment GetScalarAlignment(ShaderUniformType type) {
  switch (type) {
    case ShaderUniformType::B32:
    case ShaderUniformType::S32:
    case ShaderUniformType::U32:
    case ShaderUniformType::F32:
      return 4;
    case ShaderUniformType::F64:
      return 8;
    case ShaderUniformType::B32Vec2:
    case ShaderUniformType::B32Vec3:
    case ShaderUniformType::B32Vec4:
    case ShaderUniformType::S32Vec2:
    case ShaderUniformType::S32Vec3:
    case ShaderUniformType::S32Vec4:
    case ShaderUniformType::U32Vec2:
    case ShaderUniformType::U32Vec3:
    case ShaderUniformType::U32Vec4:
    case ShaderUniformType::Vec2:
    case ShaderUniformType::Vec3:
    case ShaderUniformType::Vec4:
      return 4;
    case ShaderUniformType::F64Vec2:
    case ShaderUniformType::F64Vec3:
    case ShaderUniformType::F64Vec4:
      return 8;
    case ShaderUniformType::Mat2x2:
    case ShaderUniformType::Mat2x3:
    case ShaderUniformType::Mat2x4:
    case ShaderUniformType::Mat3x2:
    case ShaderUniformType::Mat3x3:
    case ShaderUniformType::Mat3x4:
    case ShaderUniformType::Mat4x2:
    case ShaderUniformType::Mat4x3:
    case ShaderUniformType::Mat4x4:
      return 4;
  }

  return kInvalidAlignment;
}

// Extended alignment.
Alignment GetStd140Alignment(ShaderUniformType type) {
  switch (type) {
    case ShaderUniformType::B32:
    case ShaderUniformType::S32:
    case ShaderUniformType::U32:
    case ShaderUniformType::F32:
      return 4;
    case ShaderUniformType::F64:
      return 8;
    case ShaderUniformType::B32Vec2:
      return 8;
    case ShaderUniformType::B32Vec3:
    case ShaderUniformType::B32Vec4:
      return 16;
    case ShaderUniformType::S32Vec2:
      return 8;
    case ShaderUniformType::S32Vec3:
    case ShaderUniformType::S32Vec4:
      return 16;
    case ShaderUniformType::U32Vec2:
      return 8;
    case ShaderUniformType::U32Vec3:
    case ShaderUniformType::U32Vec4:
      return 16;
    case ShaderUniformType::Vec2:
      return 8;
    case ShaderUniformType::Vec3:
    case ShaderUniformType::Vec4:
      return 16;
    case ShaderUniformType::F64Vec2:
      return 16;
    case ShaderUniformType::F64Vec3:
    case ShaderUniformType::F64Vec4:
      return 32;
    case ShaderUniformType::Mat2x2:
    case ShaderUniformType::Mat2x3:
    case ShaderUniformType::Mat2x4:
    case ShaderUniformType::Mat3x2:
    case ShaderUniformType::Mat3x3:
    case ShaderUniformType::Mat3x4:
    case ShaderUniformType::Mat4x2:
    case ShaderUniformType::Mat4x3:
    case ShaderUniformType::Mat4x4:
      return 16;
  }

  return kInvalidAlignment;
}

// Base alignment.
Alignment GetStd430Alignment(ShaderUniformType type) {
  switch (type) {
    case ShaderUniformType::B32:
    case ShaderUniformType::S32:
    case ShaderUniformType::U32:
    case ShaderUniformType::F32:
      return 4;
    case ShaderUniformType::F64:
      return 8;
    case ShaderUniformType::B32Vec2:
      return 8;
    case ShaderUniformType::B32Vec3:
    case ShaderUniformType::B32Vec4:
      return 16;
    case ShaderUniformType::S32Vec2:
      return 8;
    case ShaderUniformType::S32Vec3:
    case ShaderUniformType::S32Vec4:
      return 16;
    case ShaderUniformType::U32Vec2:
      return 8;
    case ShaderUniformType::U32Vec3:
    case ShaderUniformType::U32Vec4:
      return 16;
    case ShaderUniformType::Vec2:
      return 8;
    case ShaderUniformType::Vec3:
    case ShaderUniformType::Vec4:
      return 16;
    case ShaderUniformType::F64Vec2:
      return 16;
    case ShaderUniformType::F64Vec3:
    case ShaderUniformType::F64Vec4:
      return 32;
    case ShaderUniformType::Mat2x2:
    case ShaderUniformType::Mat2x3:
    case ShaderUniformType::Mat2x4:
      return 8;
    case ShaderUniformType::Mat3x2:
    case ShaderUniformType::Mat3x3:
    case ShaderUniformType::Mat3x4:
    case ShaderUniformType::Mat4x2:
    case ShaderUniformType::Mat4x3:
    case ShaderUniformType::Mat4x4:
      return 16;
  }

  return kInvalidAlignment;
}

void SetName(ShaderVertexAttributeDescr& descr, const schar* name,
             uindex name_len) {
  descr.name_len = name_len;

  if (descr.name_len >= kVertexAttributeDescrMaxNameLen) {
    COMET_LOG_RENDERING_WARNING(
        "Vertex attribute name provided is too long: ", descr.name_len,
        " >= ", kVertexAttributeDescrMaxNameLen, ". It will be truncated.");
    descr.name_len = static_cast<uindex>(kVertexAttributeDescrMaxNameLen - 1);
  }

  Copy(descr.name, name, descr.name_len);
  descr.name[descr.name_len + 1] = '\0';
}

void SetName(ShaderUniformDescr& descr, const schar* name, uindex name_len) {
  descr.name_len = name_len;

  if (descr.name_len >= kShaderUniformDescrMaxNameLen) {
    COMET_LOG_RENDERING_WARNING(
        "Shader uniform name provided is too long: ", descr.name_len,
        " >= ", kShaderUniformDescrMaxNameLen, ". It will be truncated.");
    descr.name_len = static_cast<uindex>(kShaderUniformDescrMaxNameLen - 1);
  }

  Copy(descr.name, name, descr.name_len);
  descr.name[descr.name_len + 1] = '\0';
}

void GenerateGeometry(const math::Aabb& aabb,
                      std::vector<geometry::Vertex>& vertices,
                      std::vector<geometry::Index>& indices, bool is_visible) {
  COMET_ASSERT(vertices.size() == 0,
               "Tried to generate geometry for AABB, but vertices provided are "
               "not empty!");
  COMET_ASSERT(indices.size() == 0,
               "Tried to generate geometry for AABB, but indices provided are "
               "not empty!");
  const math::Vec3 extents{aabb.extents[0], aabb.extents[1], aabb.extents[2]};

  vertices.reserve(8);

  geometry::Vertex vertex{};
  vertex.color = math::Vec4{is_visible ? kColorGreen : kColorRed, 1.0f};

  // Top right far.
  constexpr auto kTopRightFarIndex{0};
  vertex.position = aabb.center + math::Vec3(extents.x, extents.y, -extents.z);
  vertices.push_back(vertex);

  // Top right near.
  constexpr auto kTopRightNearIndex{1};
  vertex.position = aabb.center + math::Vec3(extents.x, extents.y, extents.z);
  vertices.push_back(vertex);

  // Top left far.
  constexpr auto kTopLeftFarIndex{2};
  vertex.position = aabb.center + math::Vec3(-extents.x, extents.y, -extents.z);
  vertices.push_back(vertex);

  // Top left near.
  constexpr auto kTopLeftNearIndex{3};
  vertex.position = aabb.center + math::Vec3(-extents.x, extents.y, extents.z);
  vertices.push_back(vertex);

  // Bottom right far.
  constexpr auto kBottomRightFarIndex{4};
  vertex.position = aabb.center + math::Vec3(extents.x, -extents.y, -extents.z);
  vertices.push_back(vertex);

  // Bottom right near.
  constexpr auto kBottomRightNearIndex{5};
  vertex.position = aabb.center + math::Vec3(extents.x, -extents.y, extents.z);
  vertices.push_back(vertex);

  // Bottom left far.
  constexpr auto kBottomLeftFarIndex{6};
  vertex.position =
      aabb.center + math::Vec3(-extents.x, -extents.y, -extents.z);
  vertices.push_back(vertex);

  // Bottom left near.
  constexpr auto kBottomLeftNearIndex{7};
  vertex.position = aabb.center + math::Vec3(-extents.x, -extents.y, extents.z);
  vertices.push_back(vertex);

  indices.reserve(36);
  indices.push_back(kTopRightFarIndex);
  indices.push_back(kTopRightNearIndex);
  indices.push_back(kTopLeftNearIndex);

  indices.push_back(kTopRightFarIndex);
  indices.push_back(kTopLeftNearIndex);
  indices.push_back(kTopLeftFarIndex);

  indices.push_back(kTopRightFarIndex);
  indices.push_back(kBottomRightNearIndex);
  indices.push_back(kTopRightNearIndex);

  indices.push_back(kTopRightFarIndex);
  indices.push_back(kBottomRightFarIndex);
  indices.push_back(kBottomRightNearIndex);

  indices.push_back(kBottomRightFarIndex);
  indices.push_back(kBottomRightNearIndex);
  indices.push_back(kBottomLeftFarIndex);

  indices.push_back(kBottomLeftFarIndex);
  indices.push_back(kBottomRightNearIndex);
  indices.push_back(kBottomLeftNearIndex);

  indices.push_back(kTopLeftNearIndex);
  indices.push_back(kBottomLeftNearIndex);
  indices.push_back(kBottomLeftFarIndex);

  indices.push_back(kTopLeftNearIndex);
  indices.push_back(kBottomLeftFarIndex);
  indices.push_back(kTopLeftFarIndex);

  indices.push_back(kTopRightFarIndex);
  indices.push_back(kBottomRightFarIndex);
  indices.push_back(kTopLeftFarIndex);

  indices.push_back(kTopLeftFarIndex);
  indices.push_back(kBottomRightFarIndex);
  indices.push_back(kBottomLeftFarIndex);

  indices.push_back(kTopLeftNearIndex);
  indices.push_back(kTopRightNearIndex);
  indices.push_back(kBottomRightNearIndex);

  indices.push_back(kTopLeftNearIndex);
  indices.push_back(kBottomRightNearIndex);
  indices.push_back(kBottomLeftNearIndex);
}

void GenerateGeometry(const Frustum& frustum,
                      std::vector<geometry::Vertex>& vertices,
                      std::vector<geometry::Index>& indices) {
  COMET_ASSERT(
      vertices.size() == 0,
      "Tried to generate geometry for frustum, but vertices provided are "
      "not empty!");
  COMET_ASSERT(
      indices.size() == 0,
      "Tried to generate geometry for frustum, but indices provided are "
      "not empty!");

  const auto& top_face{frustum.GetTop()};
  const auto& bottom_face{frustum.GetBottom()};
  const auto& left_face{frustum.GetLeft()};
  const auto& right_face{frustum.GetRight()};
  const auto& near_face{frustum.GetNear()};
  const auto& far_face{frustum.GetFar()};
  vertices.reserve(8);

  geometry::Vertex vertex{};
  vertex.color = math::Vec4{kColorBlack, 1.0f};
  bool is_intersection{false};

  // Top right far.
  constexpr auto kTopRightFarIndex{0};
  is_intersection =
      math::Intersect(top_face, far_face, right_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Top, far and right faces in the frustum won't intersect! "
               "Frustum seems to be invalid.");
  vertices.push_back(vertex);

  // Top right near.
  constexpr auto kTopRightNearIndex{1};
  is_intersection =
      math::Intersect(top_face, near_face, right_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Top, near and right faces in the frustum won't intersect! "
               "Frustum seems to be invalid.");
  vertices.push_back(vertex);

  // Top left far.
  constexpr auto kTopLeftFarIndex{2};
  is_intersection =
      math::Intersect(top_face, far_face, left_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Top, far and left faces in the frustum won't intersect! "
               "Frustum seems to be invalid.");
  vertices.push_back(vertex);

  // Top left near.
  constexpr auto kTopLeftNearIndex{3};
  is_intersection =
      math::Intersect(top_face, near_face, left_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Top, near and left faces in the frustum won't intersect! "
               "Frustum seems to be invalid.");
  vertices.push_back(vertex);

  // Bottom right far.
  constexpr auto kBottomRightFarIndex{4};
  is_intersection =
      math::Intersect(bottom_face, far_face, right_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Bottom, far and right faces in the frustum won't intersect! "
               "Frustum seems to be invalid.");
  vertices.push_back(vertex);

  // Bottom right near.
  constexpr auto kBottomRightNearIndex{5};
  is_intersection =
      math::Intersect(bottom_face, near_face, right_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Bottom, near and right faces in the frustum won't intersect! "
               "Frustum seems to be invalid.");
  vertices.push_back(vertex);

  // Bottom left far.
  constexpr auto kBottomLeftFarIndex{6};
  is_intersection =
      math::Intersect(bottom_face, far_face, left_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Bottom, far and left faces in the frustum won't intersect! "
               "Frustum seems to be invalid.");
  vertices.push_back(vertex);

  // Bottom left near.
  constexpr auto kBottomLeftNearIndex{7};
  is_intersection =
      math::Intersect(bottom_face, near_face, left_face, vertex.position);
  COMET_ASSERT(is_intersection,
               "Bottom, near and left faces in the frustum won't intersect! "
               "Frustum seems to be invalid.");
  vertices.push_back(vertex);

  indices.reserve(36);
  indices.push_back(kTopRightFarIndex);
  indices.push_back(kTopRightNearIndex);
  indices.push_back(kTopLeftNearIndex);

  indices.push_back(kTopRightFarIndex);
  indices.push_back(kTopLeftNearIndex);
  indices.push_back(kTopLeftFarIndex);

  indices.push_back(kTopRightFarIndex);
  indices.push_back(kBottomRightNearIndex);
  indices.push_back(kTopRightNearIndex);

  indices.push_back(kTopRightFarIndex);
  indices.push_back(kBottomRightFarIndex);
  indices.push_back(kBottomRightNearIndex);

  indices.push_back(kBottomRightFarIndex);
  indices.push_back(kBottomRightNearIndex);
  indices.push_back(kBottomLeftFarIndex);

  indices.push_back(kBottomLeftFarIndex);
  indices.push_back(kBottomRightNearIndex);
  indices.push_back(kBottomLeftNearIndex);

  indices.push_back(kTopLeftNearIndex);
  indices.push_back(kBottomLeftNearIndex);
  indices.push_back(kBottomLeftFarIndex);

  indices.push_back(kTopLeftNearIndex);
  indices.push_back(kBottomLeftFarIndex);
  indices.push_back(kTopLeftFarIndex);

  indices.push_back(kTopRightFarIndex);
  indices.push_back(kBottomRightFarIndex);
  indices.push_back(kTopLeftFarIndex);

  indices.push_back(kTopLeftFarIndex);
  indices.push_back(kBottomRightFarIndex);
  indices.push_back(kBottomLeftFarIndex);

  indices.push_back(kTopLeftNearIndex);
  indices.push_back(kTopRightNearIndex);
  indices.push_back(kBottomRightNearIndex);

  indices.push_back(kTopLeftNearIndex);
  indices.push_back(kBottomRightNearIndex);
  indices.push_back(kBottomLeftNearIndex);
}
}  // namespace rendering
}  // namespace comet
