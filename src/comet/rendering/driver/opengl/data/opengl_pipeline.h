// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_PIPELINE_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_PIPELINE_H_

#include "comet/core/essentials.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace gl {
struct RasterizerState {
  bool is_wireframe{false};
  bool is_depth_bias{false};
  CullMode cull_mode{CullMode::Unknown};
};

struct DepthStencilState {
  bool is_depth_test{true};
  bool is_depth_write{true};
  CompareOp compare_op{CompareOp::Less};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet
#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_PIPELINE_H_
