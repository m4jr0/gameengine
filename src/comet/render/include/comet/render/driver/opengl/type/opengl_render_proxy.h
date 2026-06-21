// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_RENDER_PROXY_H_
#define COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_RENDER_PROXY_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/render/render_proxy.h"

namespace comet {
namespace render {
namespace gl {
struct DrawElementsIndirectCommand {
  GLuint indexCount{0};
  GLuint instanceCount{0};
  GLuint firstIndex{0};
  GLint vertexOffset{0};
  GLuint firstInstance{0};
};

struct GpuIndirectRenderProxy {
  DrawElementsIndirectCommand command{};
  RenderProxyId proxy_id{kInvalidRenderProxyId};
  BatchId batch_id{kInvalidBatchId};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_RENDER_PROXY_H_