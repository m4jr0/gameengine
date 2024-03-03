// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_DEBUG_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_DEBUG_VIEW_H_

#include "comet_precompile.h"

#include "comet/rendering/driver/opengl/handler/opengl_material_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_render_proxy_handler.h"
#include "comet/rendering/driver/opengl/view/opengl_shader_view.h"

namespace comet {
namespace rendering {
namespace gl {
struct DebugViewDescr : ShaderViewDescr {
  RenderProxyHandler* render_proxy_handler{nullptr};
};

class DebugView : public ShaderView {
 public:
  explicit DebugView(const DebugViewDescr& descr);
  DebugView(const DebugView&) = delete;
  DebugView(DebugView&&) = delete;
  DebugView& operator=(const DebugView&) = delete;
  DebugView& operator=(DebugView&&) = delete;
  virtual ~DebugView() = default;

  void Initialize() override;
  void Destroy() override;
  void Update(const ViewPacket& packet) override;

 private:
  RenderProxyHandler* render_proxy_handler_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_DEBUG_VIEW_H_
