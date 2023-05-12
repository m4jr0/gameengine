// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_VIEW_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_VIEW_HANDLER_H_

#include "comet_precompile.h"

#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_material_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_render_proxy_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_shader_handler.h"
#include "comet/rendering/driver/opengl/view/opengl_view.h"
#include "comet/rendering/rendering_common.h"
#include "comet/rendering/window/glfw/opengl/opengl_glfw_window.h"

#ifdef COMET_DEBUG
#include "comet/rendering/debugger/debugger_displayer_manager.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
namespace gl {
struct ViewHandlerDescr : HandlerDescr {
  ShaderHandler* shader_handler{nullptr};
  RenderProxyHandler* render_proxy_handler{nullptr};
#ifdef COMET_DEBUG
  DebuggerDisplayerManager* debugger_displayer_manager{nullptr};
#endif  // COMET_DEBUG
  OpenGlGlfwWindow* window{nullptr};
  std::vector<RenderingViewDescr>* rendering_view_descrs{nullptr};
};

class ViewHandler : public Handler {
 public:
  ViewHandler() = delete;
  explicit ViewHandler(const ViewHandlerDescr& descr);
  ViewHandler(const ViewHandler&) = delete;
  ViewHandler(ViewHandler&&) = delete;
  ViewHandler& operator=(const ViewHandler&) = delete;
  ViewHandler& operator=(ViewHandler&&) = delete;
  virtual ~ViewHandler() = default;

  void Initialize() override;
  void Shutdown() override;
  void Destroy(uindex view);
  void Destroy(View& view);
  void Update(const ViewPacket& packet);

  const View* Get(uindex index) const;
  const View* TryGet(uindex index) const;
  const View* Generate(const RenderingViewDescr& descr);

 private:
  View* Get(uindex index);
  View* TryGet(uindex index);
  void Destroy(View& view, bool is_destroying_handler);

  std::vector<std::unique_ptr<View>> views_{};
  ShaderHandler* shader_handler_{nullptr};
  RenderProxyHandler* render_proxy_handler_{nullptr};
#ifdef COMET_DEBUG
  DebuggerDisplayerManager* debugger_displayer_manager_{nullptr};
#endif  // COMET_DEBUG
  OpenGlGlfwWindow* window_{nullptr};
  std::vector<RenderingViewDescr>* rendering_view_descrs_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_VIEW_HANDLER_H_
