// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "rendering_common.h"

namespace comet {
namespace rendering {
std::string GetTextureTypeLabel(TextureType texture_type) {
  switch (texture_type) {
    case comet::rendering::TextureType::Unknown:
      return "unknown";
    case comet::rendering::TextureType::Ambient:
      return "ambient";
    case comet::rendering::TextureType::Diffuse:
      return "diffuse";
    case comet::rendering::TextureType::Specular:
      return "specular";
    case comet::rendering::TextureType::Height:
      return "height";
    case comet::rendering::TextureType::Color:
      return "color";
  }

  return "???";
}

std::string GetTextureFilterModeLabel(TextureFilterMode filter_mode) {
  switch (filter_mode) {
    case comet::rendering::TextureFilterMode::Unknown:
      return "unknown";
    case comet::rendering::TextureFilterMode::Linear:
      return "linear";
    case comet::rendering::TextureFilterMode::Nearest:
      return "nearest";
  }

  return "???";
}

std::string GetTextureRepeatModeLabel(TextureRepeatMode repeat_mode) {
  switch (repeat_mode) {
    case comet::rendering::TextureRepeatMode::Unknown:
      return "unknown";
    case comet::rendering::TextureRepeatMode::Repeat:
      return "repeat";
    case comet::rendering::TextureRepeatMode::MirroredRepeat:
      return "mirrored repeat";
    case comet::rendering::TextureRepeatMode::ClampToEdge:
      return "clamp to edge";
    case comet::rendering::TextureRepeatMode::ClampToBorder:
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
}  // namespace rendering
}  // namespace comet
