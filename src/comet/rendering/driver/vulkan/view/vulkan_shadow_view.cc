// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_shadow_view.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_packet.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_shader_type.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_image_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_shader_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_view_shader_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_view_utils.h"

namespace comet {
namespace rendering {
namespace vk {
ShadowView::ShadowView(const ShadowViewDescr& descr)
    : View{descr},
      shader_handler_{descr.shader_handler},
      pipeline_handler_{descr.pipeline_handler},
      render_proxy_handler_{descr.render_proxy_handler},
      lighting_handler_{descr.lighting_handler},
      mesh_handler_{descr.mesh_handler} {
  COMET_ASSERT(shader_handler_ != nullptr, "ShadowView::ShadowView",
               "shader handler is null");
  COMET_ASSERT(pipeline_handler_ != nullptr, "ShadowView::ShadowView",
               "pipeline handler is null");
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
    pipeline_handler_->Reset();
    return;
  }

  if (render_proxy_handler_->GetRenderProxyCount() == 0) {
    pipeline_handler_->Reset();
    return;
  }

  const auto command_buffer_handle{
      context_->GetFrameData().command_buffer_handle};

  UpdateShadowShaderPassData();

  for (const auto& job : *render_jobs) {
    COMET_ASSERT(job.resource != nullptr, "ShadowView::Update",
                 "shadow render job resource is null");

    TransitionShadowMapForRendering(command_buffer_handle, *job.resource,
                                    job.view_proj_index);

    VkClearValue clear_values[1]{};
    clear_values[0].depthStencil.depth = 1.0f;
    clear_values[0].depthStencil.stencil = 0;

    render_pass_handler_->BeginPass(render_pass_handle_, command_buffer_handle,
                                    job.framebuffer, clear_values, 1);

    shader_handler_->Bind(shadow_shader_, PipelineBindType::Graphics);
    PushShadowConstants(job);

    vkCmdSetDepthBias(command_buffer_handle, job.bias_constant, .0f,
                      job.bias_slope);

    SetViewportAndScissor(job.extent);
    DrawShadowCasters();

    render_pass_handler_->EndPass(command_buffer_handle);

    TransitionShadowMapForSampling(command_buffer_handle, *job.resource,
                                   job.view_proj_index);
  }

  pipeline_handler_->Reset();
}

void ShadowView::OnInitialize() {
  RenderPassDescr render_pass_descr{};
  render_pass_descr.extent = {width_, height_};
  render_pass_descr.offset = {0, 0};
  render_pass_descr.clear_flags = GenerateClearFlags(pass_descr_);

  render_pass_descr.dependencies = frame::FrameArray<VkSubpassDependency>{};
  render_pass_descr.dependencies.Reserve(1);

  auto& dependency{render_pass_descr.dependencies.EmplaceBack()};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  render_pass_descr.attachment_descrs = frame::FrameArray<AttachmentDescr>{};
  render_pass_descr.attachment_descrs.Reserve(1);
  render_pass_descr.attachment_descrs.PushBack(GenerateDepthAttachmentDescr(
      pass_descr_.depth_load_op, pass_descr_.depth_store_op,
      VK_SAMPLE_COUNT_1_BIT));

  render_pass_descr.options = kRenderPassOptionFlagBitsNone;

  render_pass_handle_ = render_pass_handler_->GetOrGenerate(render_pass_descr);
  lighting_handler_->SetRenderPass(render_pass_handle_);
  ShaderDescr shader_descr{};

  shader_descr.shader_resource_id =
      resource::GenerateResourceIdFromPath<resource::ShaderResource>(
          COMET_TCHAR("shaders/vulkan/shadow_shader.vk.cshader"));
  shader_descr.render_pass_handle = render_pass_handle_;
  shadow_shader_ = shader_handler_->GetOrGenerate(shader_descr);
}

void ShadowView::OnDestroy() {
  if (shadow_shader_) {
    shader_handler_->Destroy(shadow_shader_);
    shadow_shader_.Invalidate();
  }

  shader_handler_ = nullptr;
  pipeline_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
  lighting_handler_ = nullptr;
  mesh_handler_ = nullptr;
}

void ShadowView::UpdateShadowShaderPassData() {
  const auto frame_index{context_->GetFrameInFlightIndex()};
  const auto gpu_data{render_proxy_handler_->GetGpuData(frame_index)};

  static constexpr usize kShaderBufferBindingCapacity{3};
  auto& buffer_bindings{*COMET_FRAME_ARRAY(ShaderBufferBindingUpdate,
                                           kShaderBufferBindingCapacity)};

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
  auto& blocks{
      *COMET_FRAME_ARRAY(ShaderPushConstantBlockUpdate, kPushBlockCapacity)};

  auto& block{blocks.EmplaceBack()};
  block.block_index = 0;
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

  const auto frame_index{context_->GetFrameInFlightIndex()};

  const auto& indirect_buffer{
      render_proxy_handler_->GetShadowIndirectBuffer(frame_index)};
  COMET_ASSERT(
      indirect_buffer.handle != VK_NULL_HANDLE, "ShadowView::DrawShadowCasters",
      "shadow indirect buffer handle is invalid", "frame_index", frame_index);
  COMET_ASSERT(indirect_buffer.size > 0, "ShadowView::DrawShadowCasters",
               "shadow indirect buffer size is zero", "frame_index",
               frame_index);

  const auto command_buffer_handle{
      context_->GetFrameData().command_buffer_handle};

  mesh_handler_->Bind();
  shader_handler_->Bind(shadow_shader_, PipelineBindType::Graphics);

  vkCmdDrawIndexedIndirect(command_buffer_handle, indirect_buffer.handle, 0,
                           static_cast<u32>(indirect_batches->GetSize()),
                           sizeof(GpuIndirectRenderProxy));
}

void ShadowView::SetViewportAndScissor(VkExtent2D extent) const {
  const auto command_buffer_handle{
      context_->GetFrameData().command_buffer_handle};

  VkViewport viewport{};
  viewport.x = .0f;
  viewport.y = .0f;
  viewport.width = static_cast<f32>(extent.width);
  viewport.height = static_cast<f32>(extent.height);
  viewport.minDepth = .0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(command_buffer_handle, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = extent;
  vkCmdSetScissor(command_buffer_handle, 0, 1, &scissor);
}

void ShadowView::TransitionShadowMapForRendering(
    VkCommandBuffer command_buffer_handle, const ShadowResource& resource,
    u32 view_proj_index) const {
  TransitionShadowLayer(command_buffer_handle, resource, view_proj_index,
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        VK_ACCESS_SHADER_READ_BIT,
                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
}

void ShadowView::TransitionShadowMapForSampling(
    VkCommandBuffer command_buffer_handle, const ShadowResource& resource,
    u32 view_proj_index) const {
  TransitionShadowLayer(command_buffer_handle, resource, view_proj_index,
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                        VK_ACCESS_SHADER_READ_BIT,
                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void ShadowView::TransitionShadowLayer(
    VkCommandBuffer command_buffer_handle, const ShadowResource& resource,
    u32 view_proj_index, VkImageLayout old_layout, VkImageLayout new_layout,
    VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
    VkPipelineStageFlags src_stage_mask,
    VkPipelineStageFlags dst_stage_mask) const {
  COMET_ASSERT(view_proj_index < resource.view_proj_count,
               "ShadowView::TransitionShadowLayer",
               "shadow view projection index is out of bounds",
               "view_proj_index", view_proj_index, "view_proj_count",
               resource.view_proj_count);
  COMET_ASSERT(resource.first_layer_index >= 0,
               "ShadowView::TransitionShadowLayer",
               "shadow resource first layer index is invalid",
               "first_layer_index", resource.first_layer_index);

  const auto layer_index{static_cast<u32>(resource.first_layer_index) +
                         view_proj_index};

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = lighting_handler_->GetShadowArrayImageHandle();
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

  if (HasStencilComponent(lighting_handler_->GetShadowArrayFormat())) {
    barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  }

  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = layer_index;
  barrier.subresourceRange.layerCount = 1;

  barrier.srcAccessMask = src_access_mask;
  barrier.dstAccessMask = dst_access_mask;

  vkCmdPipelineBarrier(command_buffer_handle, src_stage_mask, dst_stage_mask, 0,
                       0, nullptr, 0, nullptr, 1, &barrier);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet