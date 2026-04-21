// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "rendering_texture_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace rendering {
const schar* GetTextureTypeLabel(TextureType type) {
  switch (type) {
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
    default:
      return kUnknownLabel;
  }
}

const schar* GetTextureRepeatModeLabel(TextureRepeatMode mode) {
  switch (mode) {
    case TextureRepeatMode::Unknown:
      return "unknown";
    case TextureRepeatMode::Repeat:
      return "repeat";
    case TextureRepeatMode::MirroredRepeat:
      return "mirrored_repeat";
    case TextureRepeatMode::ClampToEdge:
      return "clamp_to_edge";
    case TextureRepeatMode::ClampToBorder:
      return "clamp_to_border";
    default:
      return kUnknownLabel;
  }
}

const schar* GetTextureFilterModeLabel(TextureFilterMode mode) {
  switch (mode) {
    case TextureFilterMode::Unknown:
      return "unknown";
    case TextureFilterMode::Nearest:
      return "nearest";
    case TextureFilterMode::Linear:
      return "linear";
    default:
      return kUnknownLabel;
  }
}

const schar* GetTextureFormatLabel(TextureFormat format) {
  switch (format) {
    case TextureFormat::Unknown:
      return "unknown";
    case TextureFormat::Rgba8:
      return "rgba8";
    case TextureFormat::Rgb8:
      return "rgb8";
    default:
      return kUnknownLabel;
  }
}
}  // namespace rendering
}  // namespace comet