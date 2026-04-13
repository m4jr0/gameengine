// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_DEBUG_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_DEBUG_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/opengl/data/opengl_shader.h"
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
  void Update(frame::FramePacket* packet) override;

 private:
  void UpdateDebugShader(const frame::FramePacket* packet);
  void RunDebugCullGeneration();
  void DrawDebugCull();
  void SetViewport() const;

  RenderProxyHandler* render_proxy_handler_{nullptr};
  Shader* debug_shader_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_DEBUG_VIEW_H_