// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allows debugging memory leaks.
#include "../../../debug.hpp"

#include "loader.hpp"

#include <cstdio>
#include <GLFW/glfw3.h>

#include "../../../utils/logger.hpp"

// Some of this code was directly inspired from there:
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/

namespace koma {
GLuint load_bmp(const char *image_path) {
  auto logger = Logger::Get(LOGGER_KOMA_CORE_RENDERING);

  unsigned char header[54];
  unsigned int data_pos, width, height, size;
  unsigned char *data = nullptr;

  FILE *texture_file = std::fopen(image_path, "rb");

  if (!texture_file) {
    logger->Error("Could not open ", image_path);

    throw std::runtime_error(
      "An error occcurred during BMP texture loading"
    );
  }

  if (std::fread(header, 1, 54, texture_file) != 54) {
    logger->Error("Incorrect BMP file: ", image_path);
    fclose(texture_file);

    throw std::runtime_error(
      "An error occcurred during BMP texture loading"
    );
  }

  if (header[0] != 'B' || header[1] != 'M') {
    logger->Error("Incorrect BMP file: ", image_path);
    fclose(texture_file);

    throw std::runtime_error(
      "An error occcurred during BMP texture loading"
    );
  }

  if (*(int *)&(header[0x1E]) != 0) {
    logger->Error("Incorrect BMP file: ", image_path);
    fclose(texture_file);

    throw std::runtime_error(
      "An error occcurred during BMP texture loading"
    );
  }
  if (*(int *)&(header[0x1C]) != 24) {
    logger->Error("Incorrect BMP file: ", image_path);
    fclose(texture_file);

    throw std::runtime_error(
      "An error occcurred during BMP texture loading"
    );
  }

  data_pos = *(int *)&(header[0x0A]);
  size = *(int *)&(header[0x22]);
  width = *(int *)&(header[0x12]);
  height = *(int *)&(header[0x16]);

  // Malformed BMP file. Guessing information.
  if (!size) size = width * height * 3;

  if (!data_pos) data_pos = 54;

  data = new unsigned char[size];

  std::fread(data, 1, size, texture_file);
  std::fclose(texture_file);

  GLuint texture_id;

  glGenTextures(1, &texture_id);

  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    width,
    height,
    0,
    GL_BGR,
    GL_UNSIGNED_BYTE,
    data
  );

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameteri(
    GL_TEXTURE_2D,
    GL_TEXTURE_MIN_FILTER,
    GL_LINEAR_MIPMAP_LINEAR
  );

  glGenerateMipmap(GL_TEXTURE_2D);

  return texture_id;
}

#define FOURCC_DXT1 0x31545844  // Equivalent to "DXT1" in ASCII.
#define FOURCC_DXT3 0x33545844  // Equivalent to "DXT3" in ASCII.
#define FOURCC_DXT5 0x35545844  // Equivalent to "DXT5" in ASCII.

GLuint load_dds(const char *image_path) {
  unsigned char header[124];
  FILE *texture_file;

  texture_file = std::fopen(image_path, "rb");

  if (!texture_file) {
    std::cerr << "Could not open " << image_path << std::endl;

    return 0;
  }

  char file_code[4];

  std::fread(file_code, 1, 4, texture_file);

  if (strncmp(file_code, "DDS ", 4) != 0) {
    std::cerr << "Incorrect DDS file" << std::endl;

    std::fclose(texture_file);

    return 0;
  }

  std::fread(&header, 124, 1, texture_file);

  unsigned int height = *(unsigned int *)&(header[8]);
  unsigned int width = *(unsigned int *)&(header[12]);
  unsigned int linear_size = *(unsigned int *)&(header[16]);
  unsigned int mip_map_count = *(unsigned int *)&(header[24]);
  unsigned int four_cc = *(unsigned int *)&(header[80]);

  unsigned char *buffer = nullptr;
  unsigned int buffer_size;

  // How big is it going to be including all mipmaps?
  buffer_size = mip_map_count > 1 ? linear_size * 2 : linear_size;
  buffer = new unsigned char[buffer_size];

  std::fread(buffer, 1, buffer_size, texture_file);

  std::fclose(texture_file);

  unsigned int components = (four_cc == FOURCC_DXT1) ? 3 : 4;
  unsigned int format;

  switch (four_cc) {
  case FOURCC_DXT1:
    format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    break;
  case FOURCC_DXT3:
    format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    break;
  case FOURCC_DXT5:
    format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    break;
  default:
    delete buffer;

    return 0;
  }

  GLuint texture_id;
  glGenTextures(1, &texture_id);

  // "Bind" the newly created texture : all future texture functions will modify
  // this texture.
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  unsigned int block_size = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
  unsigned int offset = 0;

  // Load the mipmaps.
  for (std::size_t level = 0; level < mip_map_count && (width || height); ++level) {
    std::size_t size = ((width + 3) / 4) * ((height + 3) / 4) * block_size;
    glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset);

    offset += size;
    width /= 2;
    height /= 2;

    // Deal with Non-Power-Of-Two textures. This code is not included in the
    // webpage to reduce clutter.
    if (width < 1) width = 1;

    if (height < 1) height = 1;
  }

  delete buffer;

  return texture_id;
}
};  // namespace koma
