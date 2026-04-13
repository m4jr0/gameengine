// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_debug_view.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_packet.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_shader_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_view_shader_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_view_utils.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace rendering {
namespace vk {
DebugView::DebugView(const DebugViewDescr& descr)
    : ShaderView{descr}, render_proxy_handler_{descr.render_proxy_handler} {
  COMET_ASSERT(render_proxy_handler_ != nullptr,
               "Render proxy handler is null!");
}

void DebugView::Initialize() {
  View::Initialize();

  RenderPassDescr render_pass_descr{};

  VkExtent2D extent{};
  extent.width = width_;
  extent.height = height_;

  render_pass_descr.extent = extent;
  render_pass_descr.offset = {0, 0};

  render_pass_descr.dependencies = frame::FrameArray<VkSubpassDependency>{};
  render_pass_descr.dependencies.Reserve(1);

  auto& dependency{render_pass_descr.dependencies.EmplaceBack()};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

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

  ShaderDescr shader_descr{};
  shader_descr.shader_id =
      resource::GenerateResourceIdFromPath<resource::ShaderResource>(
          COMET_TCHAR("shaders/vulkan/forward_debug_shader.vk.cshader"));
  shader_descr.render_pass_handle = render_pass_handle_;
  debug_shader_ = shader_handler_->GetOrGenerate(shader_descr);
}

void DebugView::Destroy() {
  if (debug_shader_ != nullptr) {
    shader_handler_->Destroy(debug_shader_);
    debug_shader_ = nullptr;
  }

  render_proxy_handler_ = nullptr;
  shader_handler_ = nullptr;
  View::Destroy();
}

void DebugView::Update([[maybe_unused]] frame::FramePacket* packet) {
  COMET_PROFILE("DebugView::Update");

#ifdef COMET_DEBUG_CULLING
  if (render_proxy_handler_->GetRenderProxyCount() == 0) {
    return;
  }

  UpdateDebugShader(packet);
  RunDebugCullGeneration();

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  render_pass_handler_->BeginPass(render_pass_handle_, command_buffer_handle,
                                  context_->GetImageIndex(), nullptr, 0);

  SetViewportAndScissor();
  DrawDebugCull();

  render_pass_handler_->EndPass(command_buffer_handle);

  shader_handler_->Reset();
  pipeline_handler_->Reset();
#endif  // COMET_DEBUG_CULLING
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

  auto frame_index{context_->GetFrameInFlightIndex()};
  auto gpu_data{render_proxy_handler_->GetGpuData(frame_index)};

  auto& buffer_bindings{
      *COMET_FRAME_ARRAY(ShaderBufferBindingUpdate, static_cast<usize>(2))};

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(debug_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kDebugAabbsBinding),
      gpu_data.ssbo_debug_aabbs_handle, gpu_data.ssbo_debug_aabbs_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       debug_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kDebugLineVerticesBinding),
                   gpu_data.ssbo_debug_lines_handle,
                   gpu_data.ssbo_debug_lines_size);

  ShaderPassUpdate pass_update{};
  pass_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdatePass(debug_shader_, pass_update);
#endif  // COMET_DEBUG_CULLING
}

void DebugView::RunDebugCullGeneration() {
#ifdef COMET_DEBUG_CULLING
  auto proxy_count{render_proxy_handler_->GetRenderProxyCount()};

  if (proxy_count == 0) {
    return;
  }

  auto& blocks{
      *COMET_FRAME_ARRAY(ShaderPushConstantBlockUpdate, static_cast<usize>(1))};
  auto& block{blocks.EmplaceBack()};
  block.block_index = debugshaderconsts::kCountPushConstantIndex;
  block.data = &proxy_count;
  block.size = sizeof(proxy_count);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};

  shader_handler_->Bind(debug_shader_, PipelineBindType::Compute);
  shader_handler_->PushConstants(debug_shader_, push_constants);

  auto group_count{static_cast<u32>((proxy_count + kShaderLocalSize - 1) /
                                    kShaderLocalSize)};

  vkCmdDispatch(command_buffer_handle, group_count, 1, 1);

  const auto& debug_line_buffer{render_proxy_handler_->GetDebugLineBuffer()};

  VkBufferMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.buffer = debug_line_buffer.handle;
  barrier.offset = 0;
  barrier.size = debug_line_buffer.size;

  vkCmdPipelineBarrier(command_buffer_handle,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1,
                       &barrier, 0, nullptr);
#endif  // COMET_DEBUG_CULLING
}

void DebugView::DrawDebugCull() {
#ifdef COMET_DEBUG_CULLING
  auto vertex_count{render_proxy_handler_->GetDebugLineVertexCount()};

  if (vertex_count == 0) {
    return;
  }

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  const auto& debug_line_buffer{render_proxy_handler_->GetDebugLineBuffer()};
  const VkDeviceSize offsets[]{0};

  vkCmdBindVertexBuffers(command_buffer_handle, 0, 1, &debug_line_buffer.handle,
                         offsets);

  shader_handler_->Bind(debug_shader_, PipelineBindType::Graphics);

  vkCmdDraw(command_buffer_handle, vertex_count, 1, 0, 0);
#endif  // COMET_DEBUG_CULLING
}

void DebugView::SetViewportAndScissor() const {
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