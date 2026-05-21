// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_DEBUG_H_
#define COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_DEBUG_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/math/vector.h"
#include "comet/rendering/driver/opengl/type/opengl_storage.h"

namespace comet {
namespace rendering {
namespace gl {
struct DebugGpuData {
  GlNativeStorageHandle ssbo_debug_data_handle{kInvalidGlNativeStorageHandle};
  usize ssbo_debug_data_size{0};

  GlNativeStorageHandle ssbo_debug_lines_handle{kInvalidGlNativeStorageHandle};
  usize ssbo_debug_lines_size{0};

  GlNativeStorageHandle ssbo_debug_aabbs_handle{kInvalidGlNativeStorageHandle};
  usize ssbo_debug_aabbs_size{0};

  GlNativeStorageHandle ssbo_debug_camera_frustums_handle{
      kInvalidGlNativeStorageHandle};
  usize ssbo_debug_camera_frustums_size{0};

  GlNativeStorageHandle ssbo_debug_cascade_frustums_handle{
      kInvalidGlNativeStorageHandle};
  usize ssbo_debug_cascade_frustums_size{0};

  GlNativeStorageHandle ssbo_debug_light_frustums_handle{
      kInvalidGlNativeStorageHandle};
  usize ssbo_debug_light_frustums_size{0};
};

struct DebugFrustumCorners {
  math::Vec4 corners[8];
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_DEBUG_H_