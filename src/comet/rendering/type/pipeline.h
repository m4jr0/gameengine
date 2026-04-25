// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_TYPE_PIPELINE_H_
#define COMET_COMET_RENDERING_TYPE_PIPELINE_H_

#include "comet/core/essentials.h"

namespace comet {
namespace rendering {
enum class CullMode { Unknown = 0, None, Front, Back, FrontAndBack };

struct RasterizerDescr {
  bool is_wireframe{false};
  bool is_depth_bias{false};
  CullMode cull_mode{CullMode::Unknown};
};

enum class CompareOp : u8 {
  Unknown = 0,
  Never,
  Less,
  Equal,
  LessOrEqual,
  Greater,
  NotEqual,
  GreaterOrEqual,
  Always
};

struct DepthStencilDescr {
  bool is_depth_test{true};
  bool is_depth_write{true};
  CompareOp compare_op{CompareOp::Less};
};

enum class PrimitiveTopology {
  Unknown = 0,
  Points,
  Lines,
  LineStrip,
  Triangles,
  TriangleStrip
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_TYPE_PIPELINE_H_