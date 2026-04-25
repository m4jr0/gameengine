// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_shadow_view.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_container.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/type/opengl_shader.h"
#include "comet/rendering/driver/opengl/utils/opengl_shader_utils.h"
#include "comet/rendering/driver/opengl/utils/opengl_view_shader_utils.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader/shader_resource.h"

namespace comet {
namespace rendering {
namespace gl {
ShadowView::ShadowView(const ShadowViewDescr& descr)
    : View{descr},
      shader_handler_{descr.shader_handler},
      render_proxy_handler_{descr.render_proxy_handler},
      lighting_handler_{descr.lighting_handler},
      mesh_handler_{descr.mesh_handler} {
  COMET_ASSERT(shader_handler_ != nullptr, "ShadowView::ShadowView",
               "shader handler is null");
  COMET_ASSERT(render_proxy_handler_ != nullptr, "ShadowView::ShadowView",
               "render proxy handler is null");
  COMET_ASSERT(lighting_handler_ != nullptr, "ShadowView::ShadowView",
               "lighting handler is null");
  COMET_ASSERT(mesh_handler_ != nullptr, "ShadowView::ShadowView",
               "mesh handler is null");
}

void ShadowView::Update(frame::FramePacket*) {
  COMET_PROFILE("ShadowView::Update");
  const auto* render_jobs{lighting_handler_->GetRenderJobs()};

  if (render_jobs == nullptr || render_jobs->IsEmpty()) {
    shader_handler_->Reset();
    return;
  }

  if (render_proxy_handler_->GetRenderProxyCount() == 0) {
    shader_handler_->Reset();
    return;
  }

  UpdateShadowShaderPassData();

  GLint previous_framebuffer{0};
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previous_framebuffer);

  GLint previous_viewport[4]{};
  glGetIntegerv(GL_VIEWPORT, previous_viewport);

  GLboolean color_mask[4]{GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
  glGetBooleanv(GL_COLOR_WRITEMASK, color_mask);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glDisable(GL_BLEND);
  glEnable(GL_POLYGON_OFFSET_FILL);

  shader_handler_->Bind(shadow_shader_, ShaderBindType::Graphics);
  shader_handler_->BindVertexSource(shadow_shader_,
                                    mesh_handler_->GetVertexSource());

  for (const auto& job : *render_jobs) {
    COMET_ASSERT(job.resource != nullptr, "ShadowView::Update",
                 "shadow render job resource is null");
    COMET_ASSERT(job.framebuffer != kInvalidFrameBufferHandle,
                 "ShadowView::Update",
                 "shadow render job framebuffer is invalid");

    glBindFramebuffer(GL_FRAMEBUFFER, job.framebuffer);
    glClear(GL_DEPTH_BUFFER_BIT);

    PushShadowConstants(job);
    glPolygonOffset(job.bias_slope, job.bias_constant);
    SetViewport(job.resolution);
    DrawShadowCasters();
  }

  glDisable(GL_POLYGON_OFFSET_FILL);

  glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previous_framebuffer));
  glViewport(previous_viewport[0], previous_viewport[1], previous_viewport[2],
             previous_viewport[3]);
  glColorMask(color_mask[0], color_mask[1], color_mask[2], color_mask[3]);

  shader_handler_->Reset();
}

void ShadowView::OnInitialize() {
  ShaderDescr shader_descr{};
  shader_descr.shader_resource_id =
      resource::GenerateResourceIdFromPath<resource::ShaderResource>(
          COMET_TCHAR("shaders/opengl/shadow_shader.gl.cshader"));
  shadow_shader_ = shader_handler_->GetOrGenerate(shader_descr);
}

void ShadowView::OnDestroy() {
  if (shadow_shader_) {
    shader_handler_->Destroy(shadow_shader_);
    shadow_shader_.Invalidate();
  }

  shader_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
  mesh_handler_ = nullptr;
  lighting_handler_ = nullptr;
}

void ShadowView::UpdateShadowShaderPassData() {
  const auto frame_index{frame_state_->GetFrameInFlightIndex()};
  const auto gpu_data{render_proxy_handler_->GetGpuData(frame_index)};

  static constexpr usize kShaderBufferBindingCapacity{3};
  auto& buffer_bindings{*COMET_FRAME_ARRAY_WITH_CAPACITY(
      ShaderBufferBindingUpdate, kShaderBufferBindingCapacity)};

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       shadow_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyLocalDatasBinding),
                   gpu_data.ssbo_proxy_local_datas_handle,
                   gpu_data.ssbo_proxy_local_datas_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       shadow_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyInstancesBinding),
                   gpu_data.ssbo_proxy_instances_handle,
                   gpu_data.ssbo_proxy_instances_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       shadow_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kMatrixPalettesBinding),
                   gpu_data.ssbo_matrix_palettes_handle,
                   gpu_data.ssbo_matrix_palettes_size);

  ShaderPassUpdate pass_update{};
  pass_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdatePass(shadow_shader_, pass_update);
}

void ShadowView::PushShadowConstants(const ShadowRenderJob& job) {
  static constexpr usize kPushBlockCapacity{1};
  auto& blocks{*COMET_FRAME_ARRAY_WITH_CAPACITY(ShaderPushConstantBlockUpdate,
                                                kPushBlockCapacity)};

  auto& block{blocks.EmplaceLast()};
  block.block_index = shadowshaderconsts::kShadowLightViewProjFieldIndex;
  block.data = &job.view_proj;
  block.size = sizeof(job.view_proj);

  ShaderPushConstantsUpdate push_update{};
  push_update.blocks = &blocks;
  shader_handler_->PushConstants(shadow_shader_, push_update);
}

void ShadowView::DrawShadowCasters() {
  COMET_PROFILE("ShadowView::DrawShadowCasters");
  const auto* indirect_batches{render_proxy_handler_->GetIndirectBatches()};

  if (indirect_batches == nullptr || indirect_batches->IsEmpty()) {
    return;
  }

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};
  const auto indirect_buffer_handle{
      render_proxy_handler_->GetShadowIndirectBufferHandle(frame_index)};

  COMET_ASSERT(indirect_buffer_handle != kInvalidGlNativeStorageHandle,
               "ShadowView::DrawShadowCasters",
               "shadow indirect buffer handle is invalid");

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_buffer_handle);

  glMultiDrawElementsIndirect(shader_handler_->GetTopology(shadow_shader_),
                              GL_UNSIGNED_INT, nullptr,
                              static_cast<GLsizei>(indirect_batches->GetSize()),
                              sizeof(GpuIndirectRenderProxy));

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

void ShadowView::SetViewport(u32 resolution) const {
  glViewport(0, 0, static_cast<GLsizei>(resolution),
             static_cast<GLsizei>(resolution));
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet