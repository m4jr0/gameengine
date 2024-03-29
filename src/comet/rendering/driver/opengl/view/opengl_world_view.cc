// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_world_view.h"

namespace comet {
namespace rendering {
namespace gl {
WorldView::WorldView(const WorldViewDescr& descr)
    : ShaderView{descr}, render_proxy_handler_{descr.render_proxy_handler} {
  COMET_ASSERT(render_proxy_handler_ != nullptr,
               "Render proxy handler is null!");
}

void WorldView::Initialize() {
  View::Initialize();

  ShaderDescr shader_descr{};
  shader_descr.resource_path =
      COMET_TCHAR("shaders/opengl/default_shader.gl.cshader");
  COMET_DISALLOW_STR_ALLOC(shader_descr.resource_path);
  shader_ = shader_handler_->Generate(shader_descr);
}

void WorldView::Destroy() {
  shader_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
  View::Destroy();
}

void WorldView::Update(const ViewPacket& packet) {
  render_proxy_handler_->Update(packet.frame_count);
  shader_handler_->Bind(*shader_);
  ShaderPacket shader_packet{};
  shader_packet.frame_count = packet.frame_count;
  shader_packet.projection_matrix = &packet.projection_matrix;
  shader_packet.view_matrix = packet.view_matrix;
  shader_handler_->UpdateGlobal(*shader_, shader_packet);
  render_proxy_handler_->DrawProxies(packet.frame_count, *shader_);
  shader_handler_->Reset();
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet