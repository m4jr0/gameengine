// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_SHADOW_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_SHADOW_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/opengl/data/opengl_shader.h"
#include "comet/rendering/driver/opengl/data/opengl_shadow.h"
#include "comet/rendering/driver/opengl/handler/opengl_lighting_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_mesh_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_render_proxy_handler.h"
#include "comet/rendering/driver/opengl/view/opengl_shader_view.h"

namespace comet {
namespace rendering {
namespace gl {
struct ShadowViewDescr : ShaderViewDescr {
  RenderProxyHandler* render_proxy_handler{nullptr};
  LightingHandler* lighting_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
};

class ShadowView : public ShaderView {
 public:
  ShadowView() = delete;
  explicit ShadowView(const ShadowViewDescr& descr);
  ShadowView(const ShadowView&) = delete;
  ShadowView(ShadowView&&) = delete;
  ShadowView& operator=(const ShadowView&) = delete;
  ShadowView& operator=(ShadowView&&) = delete;
  ~ShadowView() override = default;

  void Initialize() override;
  void Destroy() override;
  void Update(frame::FramePacket*) override;

 private:
  void UpdateShadowShaderPassData();
  void PushShadowConstants(const ShadowRenderJob& job);
  void DrawShadowCasters();
  void SetViewport(u32 resolution) const;

  RenderProxyHandler* render_proxy_handler_{nullptr};
  LightingHandler* lighting_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};

  Shader* shadow_shader_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_SHADOW_VIEW_H_