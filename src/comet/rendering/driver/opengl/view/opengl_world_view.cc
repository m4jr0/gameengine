// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "opengl_world_view.h"

#include "comet/profiler/profiler.h"

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

void WorldView::Update(frame::FramePacket* packet) {
  COMET_PROFILE("WorldView::Update");

  if (!packet->is_rendering_skipped) {
    packet->draw_count = render_proxy_handler_->GetRenderProxyCount();

    // Need to bind shader here for the constants.
    shader_handler_->Bind(shader_, ShaderBindType::Compute);
    shader_handler_->UpdateGlobals(shader_, packet);
    shader_handler_->UpdateStorages(shader_, packet);
    shader_handler_->UpdateConstants(shader_, packet);

    render_proxy_handler_->Cull(shader_);

    shader_handler_->Bind(shader_, ShaderBindType::Graphics);
    shader_handler_->UpdateGlobals(shader_, packet);
    shader_handler_->UpdateStorages(shader_, packet);
    render_proxy_handler_->Draw(shader_,
                                static_cast<FrameCount>(packet->frame_count));
  }

  shader_handler_->Reset();
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet