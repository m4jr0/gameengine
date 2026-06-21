// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_VIEW_OPENGL_DEBUG_VIEW_H_
#define COMET_RENDER_DRIVER_OPENGL_VIEW_OPENGL_DEBUG_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/render/driver/opengl/handler/opengl_camera_handler.h"
#include "comet/render/driver/opengl/handler/opengl_render_proxy_handler.h"
#include "comet/render/driver/opengl/handler/opengl_shader_handler.h"
#include "comet/render/driver/opengl/view/opengl_view.h"
#include "comet/render/render_handle.h"

#ifdef COMET_DEBUG_RENDERING
#include "comet/render/driver/opengl/handler/opengl_debug_handler.h"
#endif  // COMET_DEBUG_RENDERING

namespace comet {
namespace render {
namespace gl {
struct DebugViewDescr : ViewDescr {
  CameraHandler* camera_handler{nullptr};
  ShaderHandler* shader_handler{nullptr};
  RenderProxyHandler* render_proxy_handler{nullptr};
#ifdef COMET_DEBUG_RENDERING
  DebugHandler* debug_handler{nullptr};
#endif  // COMET_DEBUG_RENDERING
};

class DebugView : public View {
 public:
  explicit DebugView(const DebugViewDescr& descr);
  DebugView(const DebugView&) = delete;
  DebugView(DebugView&&) = delete;
  DebugView& operator=(const DebugView&) = delete;
  DebugView& operator=(DebugView&&) = delete;
  ~DebugView() override = default;

  void Prepare(const ViewUpdate&) override;
  void Begin(const ViewUpdate&) override;
  void Draw(const ViewUpdate& update) override;
  void End(const ViewUpdate&) override;

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  void UpdateDebugShader();
  bool RunDebugCullGeneration();
  void DrawDebugCull(const ViewUpdate& update);

  void SetViewport(const ViewportRect& viewport) const;

  CameraHandler* camera_handler_{nullptr};
  ShaderHandler* shader_handler_{nullptr};
  RenderProxyHandler* render_proxy_handler_{nullptr};

#ifdef COMET_DEBUG_RENDERING
  DebugHandler* debug_handler_{nullptr};
#endif  // COMET_DEBUG_RENDERING

  ShaderHandle rendering_debug_shader_{};

  bool has_debug_lines_{false};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_VIEW_OPENGL_DEBUG_VIEW_H_