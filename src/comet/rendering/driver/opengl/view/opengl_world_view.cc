// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_world_view.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_packet.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/data/opengl_shader_data.h"
#include "comet/rendering/driver/opengl/utils/opengl_shader_utils.h"
#include "comet/rendering/driver/opengl/utils/opengl_view_shader_utils.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace rendering {
namespace gl {
WorldView::WorldView(const WorldViewDescr& descr)
    : View{descr},
      shadow_settings_{descr.shadow_settings},
      shader_handler_{descr.shader_handler},
      material_handler_{descr.material_handler},
      render_proxy_handler_{descr.render_proxy_handler},
      mesh_handler_{descr.mesh_handler},
      lighting_handler_{descr.lighting_handler} {
  COMET_ASSERT(shadow_settings_ != nullptr, "Shadow settings are null!");
  COMET_ASSERT(shader_handler_ != nullptr, "Shader handler is null!");
  COMET_ASSERT(material_handler_ != nullptr, "Material handler is null!");
  COMET_ASSERT(render_proxy_handler_ != nullptr,
               "Render proxy handler is null!");
  COMET_ASSERT(mesh_handler_ != nullptr, "Mesh handler is null!");
  COMET_ASSERT(lighting_handler_ != nullptr, "Lighting handler is null!");
}

void WorldView::Update(frame::FramePacket* packet) {
  COMET_PROFILE("WorldView::Update");
  COMET_ASSERT(packet != nullptr, "Frame packet is null!");

  UpdateWorldShader(packet);
  RunSparseUpload();
  RunCull(packet);

  if ((pass_descr_.flags & kViewPassFlagBitsHasColor) != 0) {
    glClearColor(clear_color_[0], clear_color_[1], clear_color_[2],
                 clear_color_[3]);
  }

  GLbitfield clear_mask{0};

  if ((pass_descr_.flags & kViewPassFlagBitsHasColor) != 0 &&
      pass_descr_.color_load_op == ViewLoadOp::Clear) {
    clear_mask |= GL_COLOR_BUFFER_BIT;
  }

  if ((pass_descr_.flags & kViewPassFlagBitsHasDepth) != 0 &&
      pass_descr_.depth_load_op == ViewLoadOp::Clear) {
    clear_mask |= GL_DEPTH_BUFFER_BIT;
  }

  if (clear_mask != 0) {
    glClear(clear_mask);
  }

  SetViewport();
  DrawWorld();

  render_proxy_handler_->Reset();
  shader_handler_->Reset();
}

void WorldView::OnInitialize() {
  {
    ShaderDescr shader_descr{};
    shader_descr.shader_resource_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/opengl/forward_shader.gl.cshader"));
    world_shader_ = shader_handler_->GetOrGenerate(shader_descr);
  }

  {
    ShaderDescr shader_descr{};
    shader_descr.shader_resource_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/opengl/forward_cull_shader.gl.cshader"));
    cull_shader_ = shader_handler_->GetOrGenerate(shader_descr);
  }

  {
    ShaderDescr shader_descr{};
    shader_descr.shader_resource_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/opengl/sparse_upload.gl.cshader"));
    sparse_upload_shader_ = shader_handler_->GetOrGenerate(shader_descr);
  }
}

void WorldView::OnDestroy() {
  if (world_shader_) {
    shader_handler_->Destroy(world_shader_);
    world_shader_.Invalidate();
  }

  if (cull_shader_) {
    shader_handler_->Destroy(cull_shader_);
    cull_shader_.Invalidate();
  }

  if (sparse_upload_shader_) {
    shader_handler_->Destroy(sparse_upload_shader_);
    sparse_upload_shader_.Invalidate();
  }

  shader_handler_ = nullptr;
  material_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
  mesh_handler_ = nullptr;
  lighting_handler_ = nullptr;
}

void WorldView::UpdateWorldShader(frame::FramePacket* packet) {
  COMET_ASSERT(packet != nullptr, "Frame packet is null!");

  packet->draw_count = render_proxy_handler_->GetRenderProxyCount();

  {
    auto& field_updates{
        *COMET_FRAME_ARRAY(ShaderBufferFieldUpdate, static_cast<usize>(6))};

    AddWorldGlobalFieldUpdates(shader_handler_, world_shader_, packet,
                               field_updates);

    AddWorldShadowSettingsFieldUpdates(shader_handler_, world_shader_,
                                       shadow_settings_, field_updates);

    ShaderGlobalUpdate global_update{};
    global_update.field_updates = &field_updates;

    const auto* shadow_map{lighting_handler_->GetShadowArrayTextureMap()};

    auto& image_bindings{
        *COMET_FRAME_ARRAY(ShaderImageBindingUpdate, static_cast<usize>(1))};
    auto& image_descriptors{
        *COMET_FRAME_ARRAY(ShaderImageDescriptor, static_cast<usize>(1))};

    AddWorldGlobalImageBindings(shader_handler_, world_shader_, shadow_map,
                                image_bindings, image_descriptors);

    global_update.image_bindings = &image_bindings;
    shader_handler_->UpdateGlobals(world_shader_, global_update);
  }

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};
  const auto gpu_data{render_proxy_handler_->GetGpuData(frame_index)};
  const auto light_gpu_data{lighting_handler_->GetLightGpuData(frame_index)};
  const auto shadow_gpu_data{lighting_handler_->GetShadowGpuData(frame_index)};

  auto& buffer_bindings{
      *COMET_FRAME_ARRAY(ShaderBufferBindingUpdate, static_cast<usize>(5))};

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       world_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyLocalDatasBinding),
                   gpu_data.ssbo_proxy_local_datas_handle,
                   gpu_data.ssbo_proxy_local_datas_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(world_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kProxyIdsBinding),
      gpu_data.ssbo_proxy_ids_handle, gpu_data.ssbo_proxy_ids_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       world_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kMatrixPalettesBinding),
                   gpu_data.ssbo_matrix_palettes_handle,
                   gpu_data.ssbo_matrix_palettes_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(world_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kLightsBinding),
      light_gpu_data.ssbo_lights_native_handle,
      light_gpu_data.ssbo_lights_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(world_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kShadowsBinding),
      shadow_gpu_data.ssbo_shadow_data_native_handle,
      shadow_gpu_data.ssbo_shadow_data_size);

  ShaderPassUpdate pass_update{};
  pass_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdatePass(world_shader_, pass_update);
}

void WorldView::RunSparseUpload() {
  COMET_PROFILE("WorldView::RunSparseUpload");

  if (!render_proxy_handler_->HasPendingSparseUpload()) {
    return;
  }

  const auto gpu_data{render_proxy_handler_->GetSparseUploadGpuData()};

  auto& buffer_bindings{
      *COMET_FRAME_ARRAY(ShaderBufferBindingUpdate, static_cast<usize>(3))};

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(
          sparse_upload_shader_, shaderconsts::kGlobalSet,
          worldsparseuploadshaderconsts::kSparseUploadWordIndicesBinding),
      gpu_data.ssbo_word_indices_handle, gpu_data.ssbo_word_indices_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(
          sparse_upload_shader_, shaderconsts::kGlobalSet,
          worldsparseuploadshaderconsts::kSparseUploadSourceWordsBinding),
      gpu_data.ssbo_source_words_handle, gpu_data.ssbo_source_words_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(
          sparse_upload_shader_, shaderconsts::kGlobalSet,
          worldsparseuploadshaderconsts::kSparseUploadDestinationWordsBinding),
      gpu_data.ssbo_destination_words_handle,
      gpu_data.ssbo_destination_words_size);

  ShaderGlobalUpdate global_update{};
  global_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdateGlobals(sparse_upload_shader_, global_update);

  auto& blocks{
      *COMET_FRAME_ARRAY(ShaderPushConstantBlockUpdate, static_cast<usize>(1))};

  auto& block{blocks.EmplaceBack()};
  block.block_index = worldsparseuploadshaderconsts::kCountPushConstantIndex;
  block.data = &gpu_data.word_count;
  block.size = sizeof(gpu_data.word_count);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;

  shader_handler_->Bind(sparse_upload_shader_, ShaderBindType::Compute);
  shader_handler_->PushConstants(sparse_upload_shader_, push_constants);

  glDispatchCompute(render_proxy_handler_->GetSparseUploadGroupCount(), 1, 1);

  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT |
                  GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void WorldView::RunCull(frame::FramePacket* packet) {
  COMET_PROFILE("WorldView::RunCull");
  const auto frame_index{frame_state_->GetFrameInFlightIndex()};
  const auto gpu_data{render_proxy_handler_->GetGpuData(frame_index)};

  COMET_ASSERT(
      gpu_data.ssbo_indirect_proxies_handle != kInvalidGlNativeStorageHandle,
      "Cull indirect buffer handle is invalid!");
  COMET_ASSERT(
      gpu_data.ssbo_proxy_instances_handle != kInvalidGlNativeStorageHandle,
      "Cull proxy instances buffer handle is invalid!");
  COMET_ASSERT(gpu_data.ssbo_proxy_ids_handle != kInvalidGlNativeStorageHandle,
               "Cull proxy IDs buffer handle is invalid!");

#ifdef COMET_DEBUG_RENDERING
  render_proxy_handler_->PrepareCullDebugWrite(frame_index);
#endif  // COMET_DEBUG_RENDERING

  packet->draw_count = render_proxy_handler_->GetRenderProxyCount();

  {
    auto& field_updates{
        *COMET_FRAME_ARRAY(ShaderBufferFieldUpdate, static_cast<usize>(5))};

    AddWorldGlobalFieldUpdates(shader_handler_, cull_shader_, packet,
                               field_updates);

    ShaderGlobalUpdate global_update{};
    global_update.field_updates = &field_updates;
    shader_handler_->UpdateGlobals(cull_shader_, global_update);
  }

  auto& buffer_bindings{
      *COMET_FRAME_ARRAY(ShaderBufferBindingUpdate, static_cast<usize>(6))};

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       cull_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyLocalDatasBinding),
                   gpu_data.ssbo_proxy_local_datas_handle,
                   gpu_data.ssbo_proxy_local_datas_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(cull_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kProxyIdsBinding),
      gpu_data.ssbo_proxy_ids_handle, gpu_data.ssbo_proxy_ids_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       cull_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyInstancesBinding),
                   gpu_data.ssbo_proxy_instances_handle,
                   gpu_data.ssbo_proxy_instances_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       cull_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kIndirectProxiesBinding),
                   gpu_data.ssbo_indirect_proxies_handle,
                   gpu_data.ssbo_indirect_proxies_size);

#ifdef COMET_DEBUG_RENDERING
  if (gpu_data.ssbo_debug_data_handle != kInvalidGlNativeStorageHandle) {
    AddBufferBinding(
        buffer_bindings,
        shader_handler_->GetBindingIndex(cull_shader_, shaderconsts::kPassSet,
                                         sharedshaderconsts::kDebugDataBinding),
        gpu_data.ssbo_debug_data_handle, gpu_data.ssbo_debug_data_size);
  }
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  if (gpu_data.ssbo_debug_aabbs_handle != kInvalidGlNativeStorageHandle) {
    AddBufferBinding(buffer_bindings,
                     shader_handler_->GetBindingIndex(
                         cull_shader_, shaderconsts::kPassSet,
                         sharedshaderconsts::kDebugAabbsBinding),
                     gpu_data.ssbo_debug_aabbs_handle,
                     gpu_data.ssbo_debug_aabbs_size);
  }
#endif  // COMET_DEBUG_CULLING

  ShaderPassUpdate pass_update{};
  pass_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdatePass(cull_shader_, pass_update);

  {
    const auto draw_count{static_cast<u32>(packet->draw_count)};

    auto& blocks{*COMET_FRAME_ARRAY(ShaderPushConstantBlockUpdate,
                                    static_cast<usize>(1))};

    auto& block{blocks.EmplaceBack()};
    block.block_index = worldcullshaderconsts::kDrawCountPushConstantIndex;
    block.data = &draw_count;
    block.size = sizeof(draw_count);

    ShaderPushConstantsUpdate push_constants{};
    push_constants.blocks = &blocks;

    shader_handler_->Bind(cull_shader_, ShaderBindType::Compute);
    shader_handler_->PushConstants(cull_shader_, push_constants);

    glDispatchCompute(render_proxy_handler_->GetCullGroupCount(), 1, 1);

    glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT |
                    GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
  }
}

void WorldView::DrawWorld() {
  COMET_PROFILE("WorldView::DrawWorld");
  const auto* batch_groups{render_proxy_handler_->GetBatchGroups()};

  if (batch_groups == nullptr || batch_groups->IsEmpty()) {
    return;
  }

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};
  const auto gpu_data{render_proxy_handler_->GetGpuData(frame_index)};

  COMET_ASSERT(
      gpu_data.ssbo_indirect_proxies_handle != kInvalidGlNativeStorageHandle,
      "Draw indirect buffer handle is invalid!");
  COMET_ASSERT(gpu_data.ssbo_proxy_ids_handle != kInvalidGlNativeStorageHandle,
               "Draw proxy IDs buffer handle is invalid!");

  const auto* indirect_batches{render_proxy_handler_->GetIndirectBatches()};

  shader_handler_->Bind(world_shader_, ShaderBindType::Graphics);
  shader_handler_->BindVertexSource(world_shader_,
                                    mesh_handler_->GetVertexSource());

  const auto light_count{lighting_handler_->GetLightCount()};

  auto& blocks{
      *COMET_FRAME_ARRAY(ShaderPushConstantBlockUpdate, static_cast<usize>(1))};

  auto& block{blocks.EmplaceBack()};
  block.block_index = worldshaderconsts::kLightingPushConstantIndex;
  block.data = &light_count;
  block.size = sizeof(light_count);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;
  shader_handler_->PushConstants(world_shader_, push_constants);

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, gpu_data.ssbo_indirect_proxies_handle);

  MaterialHandle last_material_handle{};

  for (const auto& group : *batch_groups) {
    const auto& batch{indirect_batches->Get(group.offset)};
    const auto* proxy{batch.proxy};

    if (proxy->material_handle != last_material_handle) {
      if (!shader_handler_->HasMaterial(world_shader_,
                                        proxy->material_handle)) {
        shader_handler_->BindMaterial(world_shader_, proxy->material_handle);
      }

      shader_handler_->BindInstance(world_shader_, proxy->material_handle);
      last_material_handle = proxy->material_handle;
    }

    glMultiDrawElementsIndirect(
        shader_handler_->GetTopology(world_shader_), GL_UNSIGNED_INT,
        reinterpret_cast<const void*>(group.offset *
                                      sizeof(GpuIndirectRenderProxy)),
        static_cast<GLsizei>(group.count), sizeof(GpuIndirectRenderProxy));
  }

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

void WorldView::SetViewport() const {
  glViewport(0, 0, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_));
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet