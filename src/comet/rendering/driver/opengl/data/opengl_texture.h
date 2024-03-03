// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_TEXTURE_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_TEXTURE_H_

#include "comet_precompile.h"

#include "glad/glad.h"

#include "comet/resource/resource.h"

namespace comet {
namespace rendering {
namespace gl {
using TextureHandle = u32;
constexpr auto kInvalidTextureHandle{0};

struct Texture {
  GLenum format{GL_INVALID_VALUE};
  GLenum internal_format{GL_INVALID_VALUE};
  TextureHandle handle{kInvalidTextureHandle};
  u32 width{0};
  u32 height{0};
  u32 depth{0};
  u32 mip_levels{0};
  u8 channel_count{0};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_TEXTURE_H_
