// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
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

void DebugView::Update([[maybe_unused]] frame::FramePacket* packet) {
  COMET_PROFILE("DebugView::Update");
#ifdef COMET_DEBUG_CULLING
  shader_handler_->UpdateGlobals(shader_, packet);
  shader_handler_->UpdateStorages(shader_, packet);

  shader_handler_->Bind(shader_, ShaderBindType::Compute);
  auto draw_count{render_proxy_handler_->GetRenderProxyCount()};
  shader_handler_->UpdateConstants(shader_, {nullptr, &draw_count});
  render_proxy_handler_->DebugCull(shader_);

  shader_handler_->Bind(shader_, ShaderBindType::Graphics);
  render_proxy_handler_->DrawDebugCull(shader_);
#endif  // COMET_DEBUG_CULLING
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet