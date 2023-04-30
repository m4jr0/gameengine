// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_texture.h"

namespace comet {
namespace rendering {
namespace gl {
u32 Load2DTexture(const resource::TextureResourceDescr& descr,
                  const void* pixel_data, bool is_gamma) {
  u32 texture_id{0};

  glGenTextures(1, &texture_id);

  if (pixel_data != nullptr) {
    GLenum format;

    if (descr.channel_count == 1) {
      format = GL_RED;
    } else if (descr.channel_count == 3) {
      format = GL_RGB;
    } else if (descr.channel_count == 4) {
      format = GL_RGBA;
    } else {
      COMET_LOG_RENDERING_ERROR("Unsupported texture type at path.");

      return 0;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, format, descr.resolution[0],
                 descr.resolution[1], 0, format, GL_UNSIGNED_BYTE, pixel_data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    COMET_LOG_RENDERING_ERROR("Texture failed to load at path.");
  }

  return texture_id;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
