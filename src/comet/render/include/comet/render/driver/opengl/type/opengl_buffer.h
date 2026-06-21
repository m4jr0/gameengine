// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_BUFFER_H_
#define COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_BUFFER_H_

#include "comet/core/essentials.h"
#include "comet/render/driver/opengl/type/opengl_storage.h"

namespace comet {
namespace render {
namespace gl {
struct GlBufferView {
  GlNativeStorageHandle native_handle{kInvalidGlNativeStorageHandle};
  GLsizei size{0};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_BUFFER_H_