// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/opengl/utils/opengl_texture_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type_trait.h"
#include "comet/rendering/utils/texture_utils.h"

namespace comet {
namespace rendering {
namespace gl {
HashValue GenerateHash(const TextureKey& key) {
  HashValue hash{0};
  hash = HashCombine(hash, static_cast<HashValue>(key.kind));
  hash = HashCombine(
      hash, static_cast<HashValue>(key.texture_resource_id.GetValue()));
  hash = HashCombine(hash, static_cast<HashValue>(key.runtime_id));
  hash = HashCombine(hash, static_cast<HashValue>(key.type));
  return hash;
}

GLenum GetGlWrapMode(TextureRepeatMode repeat_mode) {
  switch (repeat_mode) {
    case TextureRepeatMode::Repeat:
      return GL_REPEAT;
    case TextureRepeatMode::MirroredRepeat:
      return GL_MIRRORED_REPEAT;
    case TextureRepeatMode::ClampToEdge:
      return GL_CLAMP_TO_EDGE;
    case TextureRepeatMode::ClampToBorder:
      return GL_CLAMP_TO_BORDER;
    case TextureRepeatMode::Unknown:
    default:
      COMET_ASSERT(false, "opengl_texture_utils::GetGlWrapMode",
                   "texture repeat mode is unsupported", "repeat_mode",
                   ToUnderlying(repeat_mode));
      return GL_REPEAT;
  }
}

GLenum GetGlFilterMode(TextureFilterMode filter_mode) {
  switch (filter_mode) {
    case TextureFilterMode::Linear:
      return GL_LINEAR;
    case TextureFilterMode::Nearest:
      return GL_NEAREST;
    case TextureFilterMode::Unknown:
    default:
      COMET_ASSERT(false, "opengl_texture_utils::GetGlFilterMode",
                   "texture filter mode is unsupported", "filter_mode",
                   ToUnderlying(filter_mode));
      return GL_LINEAR;
  }
}

GLenum GetGlFormat(const resource::TextureResource* resource) {
  COMET_ASSERT(resource != nullptr, "opengl_texture_utils::GetGlFormat",
               "texture resource is null");

  switch (resource->descr.format) {
    case TextureFormat::Rgba8:
      return GL_RGBA;

    case TextureFormat::Rgb8:
      return GL_RGB;

    case TextureFormat::Unknown:
    default:
      COMET_ASSERT(false, "opengl_texture_utils::GetGlFormat",
                   "texture format is unsupported", "format",
                   ToUnderlying(resource->descr.format));
      return GL_RGBA;
  }
}

GLenum GetGlInternalFormat(const resource::TextureResource* resource,
                           TextureType type) {
  COMET_ASSERT(resource != nullptr, "opengl_texture_utils::GetGlInternalFormat",
               "texture resource is null");

  const auto is_srgb{IsSrgbTextureType(type)};

  switch (resource->descr.format) {
    case TextureFormat::Rgba8:
      return is_srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;

    case TextureFormat::Rgb8:
      return is_srgb ? GL_SRGB8 : GL_RGB8;

    case TextureFormat::Unknown:
    default:
      COMET_ASSERT(false, "opengl_texture_utils::GetGlInternalFormat",
                   "texture format is unsupported", "format",
                   ToUnderlying(resource->descr.format));
      return GL_RGBA8;
  }
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet