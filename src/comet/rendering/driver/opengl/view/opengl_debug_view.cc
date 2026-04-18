// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_debug_view.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_packet.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/data/opengl_shader_data.h"
#include "comet/rendering/driver/opengl/utils/opengl_shader_utils.h"
#include "comet/rendering/driver/opengl/utils/opengl_view_shader_utils.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace rendering {
namespace gl {
DebugView::DebugView(const DebugViewDescr& descr)
    : View{descr},
      shader_handler_{descr.shader_handler},
      render_proxy_handler_{descr.render_proxy_handler} {
  COMET_ASSERT(shader_handler_ != nullptr, "Shader handler is null!");
  COMET_ASSERT(render_proxy_handler_ != nullptr,
               "Render proxy handler is null!");
}

void DebugView::Update([[maybe_unused]] frame::FramePacket* packet) {
  COMET_PROFILE("DebugView::Update");
#ifdef COMET_DEBUG_CULLING
  COMET_ASSERT(packet != nullptr, "Frame packet is null!");

  if (render_proxy_handler_->GetRenderProxyCount() == 0) {
    return;
  }

  UpdateDebugShader(packet);
  RunDebugCullGeneration();
  SetViewport();
  DrawDebugCull();

  shader_handler_->Reset();
#endif  // COMET_DEBUG_CULLING
}

void DebugView::OnInitialize() {
  ShaderDescr shader_descr{};
  shader_descr.shader_resource_id =
      resource::GenerateResourceIdFromPath<resource::ShaderResource>(
          COMET_TCHAR("shaders/opengl/forward_debug_shader.gl.cshader"));

  debug_shader_ = shader_handler_->GetOrGenerate(shader_descr);
}

void DebugView::OnDestroy() {
  if (debug_shader_) {
    shader_handler_->Destroy(debug_shader_);
    debug_shader_.Invalidate();
  }

  shader_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
}

void DebugView::UpdateDebugShader(
    [[maybe_unused]] const frame::FramePacket* packet) {
#ifdef COMET_DEBUG_CULLING
  {
    auto& field_updates{
        *COMET_FRAME_ARRAY(ShaderBufferFieldUpdate, static_cast<usize>(2))};

    AddDebugGlobalFieldUpdates(shader_handler_, debug_shader_, packet,
                               field_updates);

    ShaderGlobalUpdate global_update{};
    global_update.field_updates = &field_updates;
    shader_handler_->UpdateGlobals(debug_shader_, global_update);
  }

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};
  const auto gpu_data{render_proxy_handler_->GetGpuData(frame_index)};

  auto& buffer_bindings{
      *COMET_FRAME_ARRAY(ShaderBufferBindingUpdate, static_cast<usize>(2))};

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(debug_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kDebugAabbsBinding),
      gpu_data.ssbo_debug_aabbs_handle,
      static_cast<usize>(gpu_data.ssbo_debug_aabbs_size));

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       debug_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kDebugLineVerticesBinding),
                   gpu_data.ssbo_debug_lines_handle,
                   static_cast<usize>(gpu_data.ssbo_debug_lines_size));

  ShaderPassUpdate pass_update{};
  pass_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdatePass(debug_shader_, pass_update);
#endif  // COMET_DEBUG_CULLING
}

void DebugView::RunDebugCullGeneration() {
#ifdef COMET_DEBUG_CULLING
  const auto proxy_count{render_proxy_handler_->GetRenderProxyCount()};

  if (proxy_count == 0) {
    return;
  }

  auto& blocks{
      *COMET_FRAME_ARRAY(ShaderPushConstantBlockUpdate, static_cast<usize>(1))};

  auto& block{blocks.EmplaceBack()};
  block.block_index = 0;
  block.data = &proxy_count;
  block.size = sizeof(proxy_count);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;

  shader_handler_->Bind(debug_shader_, ShaderBindType::Compute);
  shader_handler_->PushConstants(debug_shader_, push_constants);

  const auto group_count{static_cast<u32>((proxy_count + kShaderLocalSize - 1) /
                                          kShaderLocalSize)};

  glDispatchCompute(group_count, 1, 1);

  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT |
                  GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
#endif  // COMET_DEBUG_CULLING
}

void DebugView::DrawDebugCull() {
#ifdef COMET_DEBUG_CULLING
  const auto vertex_count{render_proxy_handler_->GetDebugLineVertexCount()};

  if (vertex_count == 0) {
    return;
  }

  const auto vertex_buffer_handle{
      render_proxy_handler_->GetDebugLineBufferHandle()};

  COMET_ASSERT(vertex_buffer_handle != kInvalidGlNativeStorageHandle,
               "Debug line buffer handle is invalid!");

  ShaderVertexSource source{};
  source.vertex_buffer_native_handle =
      render_proxy_handler_->GetDebugLineBufferHandle();
  source.has_index_buffer = false;
  source.vertex_source_id = source.vertex_buffer_native_handle;

  shader_handler_->Bind(debug_shader_, ShaderBindType::Graphics);
  shader_handler_->BindVertexSource(debug_shader_, source);

  glDrawArrays(shader_handler_->GetTopology(debug_shader_), 0,
               static_cast<GLsizei>(vertex_count));
#endif  // COMET_DEBUG_CULLING
}

void DebugView::SetViewport() const {
  glViewport(0, 0, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_));
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet