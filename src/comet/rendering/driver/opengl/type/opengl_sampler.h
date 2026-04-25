// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_SAMPLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_SAMPLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/math/vector.h"
#include "comet/rendering/rendering_handle.h"

namespace comet {
namespace rendering {
namespace gl {
using GlNativeSamplerHandle = GLuint;
constexpr auto kInvalidGlNativeSamplerHandle{
    static_cast<GlNativeSamplerHandle>(0)};

using SamplerKey = u64;

struct SamplerDescr {
  GLenum wrap_s{GL_REPEAT};
  GLenum wrap_t{GL_REPEAT};
  GLenum wrap_r{GL_REPEAT};
  GLenum min_filter{GL_LINEAR};
  GLenum mag_filter{GL_LINEAR};

  GLenum compare_mode{GL_NONE};
  GLenum compare_func{GL_LEQUAL};

  math::Vec4 border_color{.0f, .0f, .0f, .0f};
  bool use_border_color{false};
};

struct Sampler {
  SamplerKey key{0};
  GlNativeSamplerHandle native_handle{kInvalidGlNativeSamplerHandle};
  SamplerHandle handle{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_SAMPLER_H_