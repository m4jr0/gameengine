// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/opengl/view/opengl_debug_view.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_container.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/type/opengl_shader.h"
#include "comet/rendering/driver/opengl/utils/opengl_view_shader_utils.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader/shader_resource.h"

#ifdef COMET_DEBUG
#include "comet/debugging/rendering/rendering_debug_settings.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
namespace gl {
DebugView::DebugView(const DebugViewDescr& descr)
    : View{descr},
      camera_handler_{descr.camera_handler},
      shader_handler_{descr.shader_handler},
      render_proxy_handler_{descr.render_proxy_handler}
#ifdef COMET_DEBUG_RENDERING
      ,
      debug_handler_{descr.debug_handler}
#endif  // COMET_DEBUG_RENDERING
{
  COMET_ASSERT(camera_handler_ != nullptr, "DebugView::DebugView",
               "camera handler is null");
  COMET_ASSERT(shader_handler_ != nullptr, "DebugView::DebugView",
               "shader handler is null");
  COMET_ASSERT(render_proxy_handler_ != nullptr, "DebugView::DebugView",
               "render proxy handler is null");
#ifdef COMET_DEBUG_RENDERING
  COMET_ASSERT(debug_handler_ != nullptr, "DebugView::DebugView",
               "debug handler is null");
#endif  // COMET_DEBUG_RENDERING
}

void DebugView::Prepare([[maybe_unused]] const ViewUpdate& update) {
  COMET_PROFILE("DebugView::Prepare");

  has_debug_lines_ = false;

#ifdef COMET_DEBUG_RENDERING
  const auto* packet{update.packet};
  COMET_ASSERT(packet != nullptr, "DebugView::Prepare", "frame packet is null");

  bool has_debug_draw_camera{false};

  for (const auto& camera_view : *packet->camera_views) {
    if ((GetCameraFlags() & camera_view.flags) != 0 &&
        camera_view.is_debug_draw_enabled) {
      has_debug_draw_camera = true;
      break;
    }
  }

  if (!has_debug_draw_camera) {
    return;
  }

  has_debug_lines_ = debug_handler_->GetDebugLineVertexCount() > 0;

  if (has_debug_lines_) {
    UpdateDebugShader();
  }
#endif  // COMET_DEBUG_RENDERING
}

void DebugView::Begin(const ViewUpdate&) { COMET_PROFILE("DebugView::Begin"); }

void DebugView::Draw([[maybe_unused]] const ViewUpdate& update) {
  COMET_PROFILE("DebugView::Draw");

#ifdef COMET_DEBUG_RENDERING
  if (!has_debug_lines_ || !update.is_debug_draw_enabled) {
    return;
  }

  SetViewport(update.viewport);
  DrawDebugCull(update);
#endif  // COMET_DEBUG_RENDERING
}

void DebugView::End(const ViewUpdate&) { COMET_PROFILE("DebugView::End"); }

void DebugView::OnInitialize() {
  ShaderDescr shader_descr{};
  shader_descr.shader_resource_id =
      resource::GenerateResourceIdFromPath<resource::ShaderResource>(
          COMET_TCHAR("shaders/opengl/rendering_debug_draw.gl.cshader"));
  shader_descr.bind_type = PipelineBindType::Graphics;

  rendering_debug_shader_ = shader_handler_->GetOrGenerate(shader_descr);

  SetCameraFlags(kCameraFlagBitsAll);
}

void DebugView::OnDestroy() {
  if (rendering_debug_shader_) {
    shader_handler_->Destroy(rendering_debug_shader_);
    rendering_debug_shader_.Invalidate();
  }

  camera_handler_ = nullptr;
  shader_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
#ifdef COMET_DEBUG_RENDERING
  debug_handler_ = nullptr;
#endif  // COMET_DEBUG_RENDERING

  has_debug_lines_ = false;
}

void DebugView::UpdateDebugShader() {
#ifdef COMET_DEBUG_RENDERING
  const auto frame_index{frame_state_->GetFrameInFlightIndex()};

  auto& buffer_bindings{
      *COMET_FRAME_ARRAY_WITH_CAPACITY(ShaderBufferBindingUpdate, 1)};

  AddCameraBufferBinding(shader_handler_, camera_handler_,
                         rendering_debug_shader_, frame_index, buffer_bindings);

  ShaderPassUpdate pass_update{};
  pass_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdatePass(rendering_debug_shader_, pass_update);
#endif
}

void DebugView::DrawDebugCull([[maybe_unused]] const ViewUpdate& update) {
#ifdef COMET_DEBUG_RENDERING
  const auto vertex_count{debug_handler_->GetDebugLineVertexCount()};

  if (vertex_count == 0) {
    return;
  }

  const auto aabb_count{debug_handler_->GetDebugAabbCount()};
  const auto camera_frustum_count{debug_handler_->GetDebugCameraFrustumCount()};
  const auto cascade_frustum_count{
      debug_handler_->GetDebugCascadeFrustumCount()};

  struct DebugDrawPushConstants {
    u32 camera_frustum_vertex_offset{0};
    u32 cascade_frustum_vertex_offset{0};
    u32 light_frustum_vertex_offset{0};
    u32 camera_index{static_cast<u32>(-1)};
    u32 debug_draw_flags{0};
  };

#ifdef COMET_DEBUG
  const auto debug_draw_flags{
      comet::debug::RenderingDebugSettings::Get().GetDebugDrawFlags(
          update.camera_kind == CameraKind::Debug)};
#else
  const auto debug_draw_flags{kDebugDrawFlagBitsNone};
#endif  // COMET_DEBUG

  DebugDrawPushConstants push_data{};
  push_data.camera_frustum_vertex_offset = aabb_count * 24u;
  push_data.cascade_frustum_vertex_offset =
      push_data.camera_frustum_vertex_offset + camera_frustum_count * 24u;
  push_data.light_frustum_vertex_offset =
      push_data.cascade_frustum_vertex_offset + cascade_frustum_count * 24u;
  push_data.camera_index = static_cast<u32>(update.camera_index);
  push_data.debug_draw_flags = debug_draw_flags;

  auto& blocks{
      *COMET_FRAME_ARRAY_WITH_CAPACITY(ShaderPushConstantBlockUpdate, 1)};
  auto& block{blocks.EmplaceLast()};
  block.block_index = debugshaderconsts::kCountPushConstantIndex;
  block.data = &push_data;
  block.size = sizeof(push_data);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;

  ShaderVertexSource source{};
  source.vertex_buffer_native_handle =
      debug_handler_->GetDebugLineBuffer().native_handle;
  source.has_index_buffer = false;
  source.vertex_source_id = source.vertex_buffer_native_handle;

  shader_handler_->Bind(rendering_debug_shader_);
  shader_handler_->PushConstants(rendering_debug_shader_, push_constants);
  shader_handler_->BindVertexSource(rendering_debug_shader_, source);

  glDrawArrays(shader_handler_->GetTopology(rendering_debug_shader_), 0,
               static_cast<GLsizei>(vertex_count));
#endif  // COMET_DEBUG_RENDERING
}

void DebugView::SetViewport(const ViewportRect& viewport) const {
  glViewport(static_cast<GLint>(viewport.x), static_cast<GLint>(viewport.y),
             static_cast<GLsizei>(viewport.width),
             static_cast<GLsizei>(viewport.height));
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet