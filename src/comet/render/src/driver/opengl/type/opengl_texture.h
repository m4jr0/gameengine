// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_TEXTURE_H_
#define COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_TEXTURE_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/render/render_handle.h"
#include "comet/data/render/texture.h"
#include "comet/data/resource/texture/texture_resource.h"

namespace comet {
namespace render {
namespace gl {
using GlNativeTextureHandle = GLuint;
constexpr auto kInvalidGlNativeTextureHandle{
    static_cast<GlNativeTextureHandle>(0)};

enum class TextureKeyKind : u8 { Resource = 0, Runtime };

using RuntimeTextureId = u64;
constexpr auto kInvalidRuntimeTextureId{static_cast<RuntimeTextureId>(-1)};

struct TextureKey {
  TextureKeyKind kind{TextureKeyKind::Resource};
  resource::TextureResourceId texture_resource_id{};
  RuntimeTextureId runtime_id{kInvalidRuntimeTextureId};
  TextureType type{TextureType::Unknown};

  friend bool operator==(const TextureKey& lhs,
                         const TextureKey& rhs) noexcept = default;
};

HashValue GenerateHash(const TextureKey& key);

struct Texture {
  bool is_runtime{false};
  resource::TextureResourceId texture_resource_id{};
  RuntimeTextureId runtime_id{kInvalidRuntimeTextureId};

  TextureHandle handle{};
  GlNativeTextureHandle native_handle{kInvalidGlNativeTextureHandle};
  TextureType type{TextureType::Unknown};
  u32 width{0};
  u32 height{0};
  u32 depth{0};
  u32 mip_levels{1};
  u8 channel_count{0};
  GLenum format{GL_INVALID_VALUE};
  GLenum internal_format{GL_INVALID_VALUE};
  GLenum target{GL_TEXTURE_2D};
};

struct RuntimeTextureDescr {
  TextureType type{TextureType::Unknown};

  GLenum target{GL_TEXTURE_2D};

  u32 width{0};
  u32 height{0};
  u32 depth{1};

  u32 mip_levels{1};
  u8 channel_count{1};

  GLenum format{GL_INVALID_VALUE};
  GLenum internal_format{GL_INVALID_VALUE};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_TEXTURE_H_