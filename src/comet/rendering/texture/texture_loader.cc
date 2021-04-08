// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "texture_loader.h"

#define STB_IMAGE_IMPLEMENTATION

#include "GL/glew.h"
#include "comet/utils/logger.h"
#include "stb_image.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
unsigned int Load2DTextureFromFile(const std::string &texture_path,
                                   bool is_gamma) {
  unsigned int texture_id;
  int width, height, components_number;

  glGenTextures(1, &texture_id);

  auto data =
      stbi_load(texture_path.c_str(), &width, &height, &components_number, 0);

  if (data != nullptr) {
    GLenum format;

    if (components_number == 1) {
      format = GL_RED;
    } else if (components_number == 3) {
      format = GL_RGB;
    } else if (components_number == 4) {
      format = GL_RGBA;
    } else {
      Logger::Get(kLoggerCometCoreRenderTextureTextureLoader)
          ->Error("Unsupported texture type at path: ", texture_path);

      return 0;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    Logger::Get(kLoggerCometCoreRenderTextureTextureLoader)
        ->Error("Texture failed to load at path: ", texture_path);
  }

  stbi_image_free(data);

  return texture_id;
}
}  // namespace comet
