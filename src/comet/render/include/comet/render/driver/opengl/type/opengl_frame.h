// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_FRAME_H_
#define COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_FRAME_H_

#include "comet/core/essentials.h"

namespace comet {
namespace rendering {
namespace gl {
using FrameIndex = u64;
constexpr auto kInvalidFrameIndex{static_cast<FrameIndex>(-1)};

using FrameInFlightIndex = u8;
constexpr auto kInvalidFrameInFlightIndex{static_cast<FrameInFlightIndex>(-1)};

using ImageIndex = u32;
constexpr auto kInvalidImageIndex{static_cast<ImageIndex>(-1)};

struct ImageData {
  ImageIndex image_index{kInvalidImageIndex};
  ImageIndex image_count{0};
};

struct FrameData {
  bool has_upload_submission{false};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_FRAME_H_