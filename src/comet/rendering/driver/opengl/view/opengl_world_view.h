// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_WORLD_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_WORLD_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/opengl/handler/opengl_lighting_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_material_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_mesh_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_render_proxy_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_shader_handler.h"
#include "comet/rendering/driver/opengl/view/opengl_view.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/rendering/type/light.h"

namespace comet {
namespace rendering {
namespace gl {
struct WorldViewDescr : ViewDescr {
  const ShadowSettings* shadow_settings{nullptr};
  ShaderHandler* shader_handler{nullptr};
  const MaterialHandler* material_handler{nullptr};
  RenderProxyHandler* render_proxy_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
  LightingHandler* lighting_handler{nullptr};
};

class WorldView : public View {
 public:
  explicit WorldView(const WorldViewDescr& descr);
  WorldView(const WorldView&) = delete;
  WorldView(WorldView&&) = delete;
  WorldView& operator=(const WorldView&) = delete;
  WorldView& operator=(WorldView&&) = delete;
  ~WorldView() override = default;

  void Update(frame::FramePacket* packet) override;

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  void UpdateWorldShader(frame::FramePacket* packet);
  void RunSparseUpload();
  void RunCull(frame::FramePacket* packet);
  void DrawWorld();
  void SetViewport() const;

  const ShadowSettings* shadow_settings_{nullptr};

  ShaderHandler* shader_handler_{nullptr};
  const MaterialHandler* material_handler_{nullptr};
  RenderProxyHandler* render_proxy_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  LightingHandler* lighting_handler_{nullptr};

  ShaderHandle world_shader_{};
  ShaderHandle cull_shader_{};
  ShaderHandle sparse_upload_shader_{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_WORLD_VIEW_H_