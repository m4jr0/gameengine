// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_TEXTURE_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_TEXTURE_H_

#include "comet_precompile.h"

#include "glad/glad.h"

#include "comet/resource/texture_resource.h"

namespace comet {
namespace rendering {
namespace gl {
struct Texture {
  GLuint id{0};
  std::string type;
  std::string path;
};

GLuint Load2DTexture(const resource::TextureResourceDescr& descr,
                     const void* pixel_data, bool is_gamma);
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_TEXTURE_H_
