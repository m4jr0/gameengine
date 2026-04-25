// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_UTILS_OPENGL_TEXTURE_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_UTILS_OPENGL_TEXTURE_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/driver/opengl/type/opengl_texture.h"
#include "comet/rendering/type/texture.h"
#include "comet/resource/texture/texture_resource.h"

namespace comet {
namespace rendering {
namespace gl {
HashValue GenerateHash(const TextureKey& key);

GLenum GetGlWrapMode(TextureRepeatMode repeat_mode);
GLenum GetGlFilterMode(TextureFilterMode filter_mode);
GLenum GetGlFormat(const resource::TextureResource* resource);
GLenum GetGlInternalFormat(const resource::TextureResource* resource,
                           TextureType type);
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_UTILS_OPENGL_TEXTURE_UTILS_H_