// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_RENDER_TEXTURE_TEXTURE_LOADER_HPP_
#define KOMA_CORE_RENDER_TEXTURE_TEXTURE_LOADER_HPP_

#define LOGGER_KOMA_CORE_RENDER "koma_core_render"

#define FOURCC_DXT1 0x31545844  // Equivalent to "DXT1" in ASCII.
#define FOURCC_DXT3 0x33545844  // Equivalent to "DXT3" in ASCII.
#define FOURCC_DXT5 0x35545844  // Equivalent to "DXT5" in ASCII.

#include <GL/glew.h>

// Some of this code was directly inspired from there:
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/

namespace koma {
  GLuint load_bmp(const char *);
  GLuint load_dds(const char *);
};  // namespace koma

#endif  // KOMA_CORE_RENDER_TEXTURE_TEXTURE_LOADER_HPP_
