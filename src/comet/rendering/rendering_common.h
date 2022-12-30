// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_COMMON_H_
#define COMET_COMET_RENDERING_RENDERING_COMMON_H_

#include "comet_precompile.h"

#include "glm/glm.hpp"

namespace comet {
namespace rendering {
using WindowSize = u16;

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 bitangent;
  glm::vec2 uv;
  glm::vec3 color;
};

using Index = u32;

enum class TextureType : u32 {
  Unknown = 0,
  Ambient,
  Diffuse,
  Specular,
  Height,
  Color
};

enum class TextureFormat : u32 { Unknown = 0, Rgba8, Rgb8 };
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_COMMON_H_
