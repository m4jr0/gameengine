// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_FRAME_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_FRAME_H_

#include "comet_precompile.h"

namespace comet {
namespace rendering {
namespace gl {
using FrameIndex = u64;
constexpr auto kInvalidFrameIndex{static_cast<FrameIndex>(-1)};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_FRAME_H_