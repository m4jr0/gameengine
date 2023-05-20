// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_PROXY_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_PROXY_HANDLER_H_

#include "comet_precompile.h"

#include "comet/rendering/driver/opengl/data/opengl_frame.h"
#include "comet/rendering/driver/opengl/data/opengl_material.h"
#include "comet/rendering/driver/opengl/data/opengl_mesh.h"
#include "comet/rendering/driver/opengl/data/opengl_render_proxy.h"
#include "comet/rendering/driver/opengl/data/opengl_shader.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_material_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_mesh_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_shader_handler.h"

namespace comet {
namespace rendering {
namespace gl {
struct RenderProxyHandlerDescr : HandlerDescr {
  MaterialHandler* material_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
  ShaderHandler* shader_handler{nullptr};
};

class RenderProxyHandler : public Handler {
 public:
  RenderProxyHandler() = delete;
  explicit RenderProxyHandler(const RenderProxyHandlerDescr& descr);
  RenderProxyHandler(const RenderProxyHandler&) = delete;
  RenderProxyHandler(RenderProxyHandler&&) = delete;
  RenderProxyHandler& operator=(const RenderProxyHandler&) = delete;
  RenderProxyHandler& operator=(RenderProxyHandler&&) = delete;
  virtual ~RenderProxyHandler() = default;

  void Shutdown() override;

  void Update(FrameIndex frame_count, time::Interpolation interpolation);
  void DrawProxies(FrameIndex frame_count, Shader& shader);
  // TODO(m4jr0): Remove temporary code.
  void DrawProxiesForDebugging(FrameIndex frame_count, Shader& shader);
  u32 GetDrawCount() const noexcept;

 private:
  RenderProxy GenerateInternal(Mesh& mesh, Material& material,
                               const math::Mat4& transform);
  void Draw(FrameIndex frame_count, const RenderProxy& proxy);

  constexpr static auto kDefaultProxyCount{512};
  FrameIndex update_frame_{kInvalidFrameIndex};
  std::vector<RenderProxy> proxies_{};
  const Mesh* last_drawn_mesh_{nullptr};
  MaterialHandler* material_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  ShaderHandler* shader_handler_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_PROXY_HANDLER_H_
