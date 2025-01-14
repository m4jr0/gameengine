// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "opengl_debug_view.h"

#include "comet/profiler/profiler.h"

namespace comet {
namespace rendering {
namespace gl {
DebugView::DebugView(const DebugViewDescr& descr)
    : ShaderView{descr}, render_proxy_handler_{descr.render_proxy_handler} {
  COMET_ASSERT(render_proxy_handler_ != nullptr,
               "Render proxy handler is null!");
}

void DebugView::Initialize() {
  View::Initialize();

  ShaderDescr shader_descr{};
  shader_descr.resource_path =
      COMET_TCHAR("shaders/opengl/debug_shader.gl.cshader");
  shader_ = shader_handler_->Generate(shader_descr);
}

void DebugView::Destroy() {
  shader_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
  View::Destroy();
}

void DebugView::Update(const ViewPacket& packet) {
  COMET_PROFILE("DebugView::Update");
  render_proxy_handler_->Update(packet.frame_count);
  shader_handler_->Bind(*shader_);
  ShaderPacket shader_packet{};
  shader_packet.projection_matrix = &packet.projection_matrix;
  shader_packet.view_matrix = packet.view_matrix;
  shader_handler_->UpdateGlobal(*shader_, shader_packet);
  // TODO(m4jr0): Remove temporary code.
  render_proxy_handler_->DrawProxiesForDebugging(*shader_);
  shader_handler_->Reset();
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet