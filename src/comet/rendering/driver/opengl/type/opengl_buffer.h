// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_BUFFER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_BUFFER_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/opengl/type/opengl_storage.h"

namespace comet {
namespace rendering {
namespace gl {
struct GlBufferView {
  GlNativeStorageHandle native_handle{kInvalidGlNativeStorageHandle};
  GLsizei size{0};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_BUFFER_H_