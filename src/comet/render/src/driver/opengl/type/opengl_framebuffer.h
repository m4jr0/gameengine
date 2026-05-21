// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_FRAMEBUFFER_H_
#define COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_FRAMEBUFFER_H_

#include "comet/core/essentials.h"

namespace comet {
namespace rendering {
namespace gl {
using FrameBufferHandle = GLuint;
constexpr FrameBufferHandle kInvalidFrameBufferHandle{0};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_FRAMEBUFFER_H_