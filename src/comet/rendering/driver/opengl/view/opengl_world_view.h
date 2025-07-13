// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_WORLD_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_WORLD_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/rendering/driver/opengl/handler/opengl_render_proxy_handler.h"
#include "comet/rendering/driver/opengl/view/opengl_shader_view.h"

namespace comet {
namespace rendering {
namespace gl {
struct WorldViewDescr : ShaderViewDescr {
  RenderProxyHandler* render_proxy_handler{nullptr};
};

class WorldView : public ShaderView {
 public:
  explicit WorldView(const WorldViewDescr& descr);
  WorldView(const WorldView&) = delete;
  WorldView(WorldView&&) = delete;
  WorldView& operator=(const WorldView&) = delete;
  WorldView& operator=(WorldView&&) = delete;
  virtual ~WorldView() = default;

  void Initialize() override;
  void Destroy() override;
  void Update(frame::FramePacket* packet) override;

 private:
  RenderProxyHandler* render_proxy_handler_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_WORLD_VIEW_H_
