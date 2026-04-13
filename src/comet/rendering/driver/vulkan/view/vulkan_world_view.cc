// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_world_view.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_packet.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader_data.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_shader_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_view_shader_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_view_utils.h"

namespace comet {
namespace rendering {
namespace vk {
WorldView::WorldView(const WorldViewDescr& descr)
    : ShaderView{descr},
      shadow_settings_{descr.shadow_settings},
      render_proxy_handler_{descr.render_proxy_handler},
      mesh_handler_{descr.mesh_handler},
      lighting_handler_{descr.lighting_handler} {
  COMET_ASSERT(shadow_settings_ != nullptr, "Shadow settings are null!");
  COMET_ASSERT(render_proxy_handler_ != nullptr,
               "Render proxy handler is null!");
  COMET_ASSERT(mesh_handler_ != nullptr, "Mesh handler is null!");
  COMET_ASSERT(lighting_handler_ != nullptr, "Lighting handler is null!");
}

void WorldView::Initialize() {
  View::Initialize();

  RenderPassDescr render_pass_descr{};

  VkExtent2D extent{};
  extent.width = width_;
  extent.height = height_;

  render_pass_descr.extent = extent;
  render_pass_descr.offset = {0, 0};

  render_pass_descr.dependencies = frame::FrameArray<VkSubpassDependency>{};
  render_pass_descr.dependencies.Reserve(2);

  auto& dependency_1{render_pass_descr.dependencies.EmplaceBack()};
  dependency_1.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency_1.dstSubpass = 0;
  dependency_1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency_1.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency_1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency_1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency_1.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  auto& dependency_2{render_pass_descr.dependencies.EmplaceBack()};
  dependency_2.srcSubpass = 0;
  dependency_2.dstSubpass = VK_SUBPASS_EXTERNAL;
  dependency_2.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency_2.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
  dependency_2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency_2.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  dependency_2.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  const auto& device{context_->GetDevice()};
  auto is_msaa{device.IsMsaa()};

  render_pass_descr.clear_flags = GenerateClearFlags(pass_descr_);
  render_pass_descr.attachment_descrs = frame::FrameArray<AttachmentDescr>{};
  render_pass_descr.attachment_descrs.Reserve(is_msaa ? 3 : 2);

  GenerateAttachmentDescrs(
      pass_descr_, is_msaa ? device.GetMsaaSamples() : VK_SAMPLE_COUNT_1_BIT,
      render_pass_descr.attachment_descrs);

  render_pass_descr.options = kRenderPassOptionFlagBitsSwapchainTarget;

  if (is_msaa) {
    render_pass_descr.options |= kRenderPassOptionFlagBitsMultisampled;
  }

  render_pass_handle_ = render_pass_handler_->GetOrGenerate(render_pass_descr);

  {
    ShaderDescr shader_descr{};
    shader_descr.shader_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/vulkan/forward_shader.vk.cshader"));
    shader_descr.render_pass_handle = render_pass_handle_;
    world_shader_ = shader_handler_->GetOrGenerate(shader_descr);
  }

  {
    ShaderDescr shader_descr{};
    shader_descr.shader_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/vulkan/forward_cull_shader.vk.cshader"));
    shader_descr.render_pass_handle = render_pass_handle_;
    cull_shader_ = shader_handler_->GetOrGenerate(shader_descr);
  }

  {
    ShaderDescr shader_descr{};
    shader_descr.shader_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/vulkan/sparse_upload.vk.cshader"));
    shader_descr.render_pass_handle = render_pass_handle_;
    sparse_upload_shader_ = shader_handler_->GetOrGenerate(shader_descr);
  }
}

void WorldView::Destroy() {
  if (world_shader_ != nullptr) {
    shader_handler_->Destroy(world_shader_);
    world_shader_ = nullptr;
  }

  if (cull_shader_ != nullptr) {
    shader_handler_->Destroy(cull_shader_);
    cull_shader_ = nullptr;
  }

  if (sparse_upload_shader_ != nullptr) {
    shader_handler_->Destroy(sparse_upload_shader_);
    sparse_upload_shader_ = nullptr;
  }

  render_proxy_handler_ = nullptr;
  shader_handler_ = nullptr;
  mesh_handler_ = nullptr;
  lighting_handler_ = nullptr;
  View::Destroy();
}

void WorldView::Update(frame::FramePacket* packet) {
  COMET_PROFILE("WorldView::Update");
  UpdateWorldShader(packet);
  RunSparseUpload();
  RunCull(packet);

  VkClearValue clear_values[2]{};
  memory::CopyMemory(&clear_values[0].color, clear_color_,
                     sizeof(clear_values[0].color.float32[0]) * 4);
  clear_values[1].depthStencil.depth = 1.0f;
  clear_values[1].depthStencil.stencil = 0;

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  render_pass_handler_->BeginPass(render_pass_handle_, command_buffer_handle,
                                  context_->GetImageIndex(), clear_values, 2);

  SetViewportAndScissor();
  DrawWorld();

  render_pass_handler_->EndPass(command_buffer_handle);

  render_proxy_handler_->Reset();
  shader_handler_->Reset();
  pipeline_handler_->Reset();
}

void WorldView::UpdateWorldShader(frame::FramePacket* packet) {
  packet->draw_count = render_proxy_handler_->GetRenderProxyCount();

  {
    static constexpr usize kShaderBufferFieldCapacity{6};
    auto& field_updates{*COMET_FRAME_ARRAY(ShaderBufferFieldUpdate,
                                           kShaderBufferFieldCapacity)};

    AddWorldGlobalFieldUpdates(shader_handler_, world_shader_, packet,
                               field_updates);

    AddWorldShadowSettingsFieldUpdates(shader_handler_, world_shader_,
                                       shadow_settings_, field_updates);

    ShaderGlobalUpdate global_update{};
    global_update.field_updates = &field_updates;

    const auto* shadow_map{lighting_handler_->GetShadowArrayTextureMap()};

    static constexpr usize kShaderImageBindingCapacity{1};
    auto& image_bindings{*COMET_FRAME_ARRAY(ShaderImageBindingUpdate,
                                            kShaderImageBindingCapacity)};
    auto& image_descriptors{
        *COMET_FRAME_ARRAY(ShaderImageDescriptor, kShaderImageBindingCapacity)};

    AddWorldGlobalImageBindings(shader_handler_, world_shader_, shadow_map,
                                image_bindings, image_descriptors);
    global_update.image_bindings = &image_bindings;

    shader_handler_->UpdateGlobals(world_shader_, global_update);
  }

  auto frame_index{context_->GetFrameInFlightIndex()};
  auto proxy_gpu_data{render_proxy_handler_->GetGpuData(frame_index)};
  auto light_gpu_data{lighting_handler_->GetLightGpuData(frame_index)};
  auto shadow_gpu_data{lighting_handler_->GetShadowGpuData(frame_index)};

  static constexpr usize kShaderBufferBindingCapacity{5};
  auto& buffer_bindings{*COMET_FRAME_ARRAY(ShaderBufferBindingUpdate,
                                           kShaderBufferBindingCapacity)};

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       world_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyLocalDatasBinding),
                   proxy_gpu_data.ssbo_proxy_local_datas_handle,
                   proxy_gpu_data.ssbo_proxy_local_datas_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(world_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kProxyIdsBinding),
      proxy_gpu_data.ssbo_proxy_ids_handle, proxy_gpu_data.ssbo_proxy_ids_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       world_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kMatrixPalettesBinding),
                   proxy_gpu_data.ssbo_matrix_palettes_handle,
                   proxy_gpu_data.ssbo_matrix_palettes_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(world_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kLightsBinding),
      light_gpu_data.ssbo_lights_handle, light_gpu_data.ssbo_lights_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(world_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kShadowsBinding),
      shadow_gpu_data.ssbo_shadow_data_handle,
      shadow_gpu_data.ssbo_shadow_data_size);

  ShaderPassUpdate pass_update{};
  pass_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdatePass(world_shader_, pass_update);
}

void WorldView::RunSparseUpload() {
  if (!render_proxy_handler_->HasPendingSparseUpload()) {
    return;
  }

  auto gpu_data{render_proxy_handler_->GetSparseUploadGpuData()};

  static constexpr usize kShaderBufferBindingCapacity{3};
  auto& buffer_bindings{*COMET_FRAME_ARRAY(ShaderBufferBindingUpdate,
                                           kShaderBufferBindingCapacity)};

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

  static constexpr usize kShaderPushConstantBlockCapacity{1};
  auto& blocks{*COMET_FRAME_ARRAY(ShaderPushConstantBlockUpdate,
                                  kShaderPushConstantBlockCapacity)};
  auto& block{blocks.EmplaceBack()};
  block.block_index = worldsparseuploadshaderconsts::kCountPushConstantIndex;
  block.data = &gpu_data.word_count;
  block.size = sizeof(gpu_data.word_count);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;

  shader_handler_->Bind(sparse_upload_shader_, PipelineBindType::Compute);
  shader_handler_->PushConstants(sparse_upload_shader_, push_constants);

  vkCmdDispatch(context_->GetFrameData().command_buffer_handle,
                render_proxy_handler_->GetSparseUploadGroupCount(), 1, 1);

  auto& barriers{
      *COMET_FRAME_ARRAY(VkBufferMemoryBarrier, static_cast<usize>(1))};
  render_proxy_handler_->PopulateSparseUploadBarriers(barriers);

  ApplyBufferMemoryBarriers(barriers,
                            context_->GetFrameData().command_buffer_handle,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
}

void WorldView::RunCull(frame::FramePacket* packet) {
  auto frame_index{context_->GetFrameInFlightIndex()};
  auto gpu_data{render_proxy_handler_->GetGpuData(frame_index)};

#ifdef COMET_DEBUG_RENDERING
  render_proxy_handler_->PrepareCullDebugWrite(frame_index);
#endif  // COMET_DEBUG_RENDERING

  packet->draw_count = render_proxy_handler_->GetRenderProxyCount();

  {
    static constexpr usize kShaderBufferFieldCapacity{5};
    auto& field_updates{*COMET_FRAME_ARRAY(ShaderBufferFieldUpdate,
                                           kShaderBufferFieldCapacity)};
    AddWorldGlobalFieldUpdates(shader_handler_, cull_shader_, packet,
                               field_updates);

    ShaderGlobalUpdate global_update{};
    global_update.field_updates = &field_updates;
    shader_handler_->UpdateGlobals(cull_shader_, global_update);
  }

  static constexpr usize kShaderBufferBindingCapacity{6};
  auto& buffer_bindings{*COMET_FRAME_ARRAY(ShaderBufferBindingUpdate,
                                           kShaderBufferBindingCapacity)};

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
  if (gpu_data.ssbo_debug_data_handle != VK_NULL_HANDLE) {
    AddBufferBinding(
        buffer_bindings,
        shader_handler_->GetBindingIndex(cull_shader_, shaderconsts::kPassSet,
                                         sharedshaderconsts::kDebugDataBinding),
        gpu_data.ssbo_debug_data_handle, gpu_data.ssbo_debug_data_size);
  }
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  if (gpu_data.ssbo_debug_aabbs_handle != VK_NULL_HANDLE) {
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

  static constexpr usize kShaderPushConstantBlockCapacity{1};
  auto& blocks{*COMET_FRAME_ARRAY(ShaderPushConstantBlockUpdate,
                                  kShaderPushConstantBlockCapacity)};

  auto draw_count{static_cast<u32>(packet->draw_count)};
  auto& block{blocks.EmplaceBack()};
  block.block_index = worldcullshaderconsts::kDrawCountPushConstantIndex;
  block.data = &draw_count;
  block.size = sizeof(draw_count);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;

  shader_handler_->Bind(cull_shader_, PipelineBindType::Compute);
  shader_handler_->PushConstants(cull_shader_, push_constants);

  vkCmdDispatch(context_->GetFrameData().command_buffer_handle,
                render_proxy_handler_->GetCullGroupCount(), 1, 1);

  {
    auto& barriers{
        *COMET_FRAME_ARRAY(VkBufferMemoryBarrier, static_cast<usize>(2))};
    render_proxy_handler_->PopulateDrawCullBarriers(frame_index, barriers);

    ApplyBufferMemoryBarriers(barriers,
                              context_->GetFrameData().command_buffer_handle,
                              VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                              VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
                                  VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
  }

#ifdef COMET_DEBUG_RENDERING
  {
    auto& barriers{
        *COMET_FRAME_ARRAY(VkBufferMemoryBarrier, static_cast<usize>(1))};
    render_proxy_handler_->PopulateCullDebugReadBarriers(frame_index, barriers);

    ApplyBufferMemoryBarriers(
        barriers, context_->GetFrameData().command_buffer_handle,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT);
  }
#endif  // COMET_DEBUG_RENDERING
}

void WorldView::DrawWorld() {
  const auto* batch_groups{render_proxy_handler_->GetBatchGroups()};

  if (batch_groups == nullptr || batch_groups->IsEmpty()) {
    return;
  }

  auto frame_index{context_->GetFrameInFlightIndex()};
  const auto& indirect_buffer{
      render_proxy_handler_->GetIndirectBuffer(frame_index)};
  const auto* indirect_batches{render_proxy_handler_->GetIndirectBatches()};

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};

  mesh_handler_->Bind();
  shader_handler_->Bind(world_shader_, PipelineBindType::Graphics);

  auto light_count{lighting_handler_->GetLightCount()};

  static constexpr usize kShaderPushConstantBlockCapacity{1};
  auto& blocks{*COMET_FRAME_ARRAY(ShaderPushConstantBlockUpdate,
                                  kShaderPushConstantBlockCapacity)};
  auto& block{blocks.EmplaceBack()};
  block.block_index = worldshaderconsts::kLightingPushConstantIndex;
  block.data = &light_count;
  block.size = sizeof(light_count);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;
  shader_handler_->PushConstants(world_shader_, push_constants);

  MaterialId last_mat_id{kInvalidMaterialId};

  for (const auto& group : *batch_groups) {
    const auto& batch{indirect_batches->Get(group.offset)};
    const auto* proxy{batch.proxy};

    if (proxy->mat_id != last_mat_id) {
      auto* material{material_handler_->Get(proxy->mat_id)};

      if (!shader_handler_->HasMaterial(world_shader_, material)) {
        shader_handler_->BindMaterial(world_shader_, material);
      }

      shader_handler_->BindInstance(world_shader_, material,
                                    PipelineBindType::Graphics);
      last_mat_id = proxy->mat_id;
    }

    vkCmdDrawIndexedIndirect(command_buffer_handle, indirect_buffer.handle,
                             group.offset * sizeof(GpuIndirectRenderProxy),
                             group.count, sizeof(GpuIndirectRenderProxy));
  }
}

void WorldView::SetViewportAndScissor() const {
  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};

  VkViewport viewport{};
  viewport.x = .0f;
  viewport.y = .0f;
  viewport.width = width_;
  viewport.height = height_;
  viewport.minDepth = .0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(command_buffer_handle, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = {width_, height_};
  vkCmdSetScissor(command_buffer_handle, 0, 1, &scissor);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet