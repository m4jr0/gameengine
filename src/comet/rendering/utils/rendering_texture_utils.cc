// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "rendering_texture_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/math/math_scalar.h"

namespace comet {
namespace rendering {
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
  COMET_ASSERT(width > 0 || height > 0, "rendering_texture_utils::GetMipLevels",
               "texture dimensions are zero", "width", width, "height", height);

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