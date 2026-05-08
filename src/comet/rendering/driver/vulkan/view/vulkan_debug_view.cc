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

#include "comet/core/frame/frame_container.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_shader.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_view_shader_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_view_utils.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader/shader_resource.h"

#ifdef COMET_DEBUG
#include "comet/debugging/rendering/rendering_debug_settings.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
namespace vk {
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

void DebugView::Prepare(const ViewUpdate& update) {
  COMET_PROFILE("DebugView::Prepare");

  has_debug_lines_ = false;
  is_pass_open_ = false;

  [[maybe_unused]] const auto* packet{update.packet};
  COMET_ASSERT(packet != nullptr, "DebugView::Prepare", "frame packet is null");

#ifdef COMET_DEBUG_RENDERING
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

void DebugView::Begin(const ViewUpdate&) {
  COMET_PROFILE("DebugView::Begin");

#ifdef COMET_DEBUG_RENDERING
  if (!has_debug_lines_) {
    return;
  }

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};
  const auto command_buffer_handle{frame_data.command_buffer_handle};

  render_pass_handler_->BeginPass(render_pass_handle_, command_buffer_handle,
                                  context_->GetImageIndex(), nullptr, 0);

  is_pass_open_ = true;
#endif  // COMET_DEBUG_RENDERING
}

void DebugView::Draw([[maybe_unused]] const ViewUpdate& update) {
  COMET_PROFILE("DebugView::Draw");

#ifdef COMET_DEBUG_RENDERING
  if (!is_pass_open_ || !update.is_debug_draw_enabled) {
    return;
  }

  SetViewportAndScissor(update.viewport);
  DrawDebugCull(update);
#endif  // COMET_DEBUG_RENDERING
}

void DebugView::End([[maybe_unused]] const ViewUpdate& update) {
  COMET_PROFILE("DebugView::End");

#ifdef COMET_DEBUG_RENDERING
  if (!is_pass_open_) {
    return;
  }

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};
  const auto command_buffer_handle{frame_data.command_buffer_handle};

  render_pass_handler_->EndPass(command_buffer_handle);
  is_pass_open_ = false;
#endif  // COMET_DEBUG_RENDERING
}

void DebugView::OnInitialize() {
  RenderPassDescr render_pass_descr{};

  VkExtent2D extent{};
  extent.width = width_;
  extent.height = height_;

  render_pass_descr.extent = extent;
  render_pass_descr.offset = {0, 0};

  render_pass_descr.dependencies = frame::FrameArray<VkSubpassDependency>{};
  render_pass_descr.dependencies.Reserve(1);

  auto& dependency{render_pass_descr.dependencies.EmplaceLast()};
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
  const auto is_msaa{device.IsMsaa()};

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
    shader_descr.shader_resource_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/vulkan/rendering_debug_draw.vk.cshader"));
    shader_descr.render_pass_handle = render_pass_handle_;
    shader_descr.bind_type = PipelineBindType::Graphics;
    rendering_debug_shader_ = shader_handler_->GetOrGenerate(shader_descr);
  }

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
  has_debug_lines_ = false;
  is_pass_open_ = false;
}

void DebugView::UpdateDebugShader() {
#ifdef COMET_DEBUG_RENDERING
  const auto frame_index{context_->GetFrameInFlightIndex()};

  auto& buffer_bindings{
      *COMET_FRAME_ARRAY_WITH_CAPACITY(ShaderBufferBindingUpdate, 1)};

  AddCameraBufferBinding(shader_handler_, camera_handler_,
                         rendering_debug_shader_, frame_index, buffer_bindings);

  ShaderPassUpdate pass_update{};
  pass_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdatePass(rendering_debug_shader_, pass_update);
#endif  // COMET_DEBUG_RENDERING
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

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};
  const auto command_buffer_handle{frame_data.command_buffer_handle};
  const auto& debug_line_buffer{debug_handler_->GetDebugLineBuffer()};
  const VkDeviceSize offsets[]{0};

  vkCmdBindVertexBuffers(command_buffer_handle, 0, 1, &debug_line_buffer.handle,
                         offsets);

  shader_handler_->Bind(rendering_debug_shader_);
  shader_handler_->PushConstants(rendering_debug_shader_, push_constants);

  vkCmdDraw(command_buffer_handle, vertex_count, 1, 0, 0);
#endif  // COMET_DEBUG_RENDERING
}

void DebugView::SetViewportAndScissor(const ViewportRect& viewport) const {
  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};
  const auto command_buffer_handle{frame_data.command_buffer_handle};

  VkViewport vk_viewport{
      .x = viewport.x,
      .y = viewport.y,
      .width = viewport.width,
      .height = viewport.height,
      .minDepth = .0f,
      .maxDepth = 1.0f,
  };

  vkCmdSetViewport(command_buffer_handle, 0, 1, &vk_viewport);

  VkRect2D scissor{
      .offset =
          {
              static_cast<s32>(viewport.x),
              static_cast<s32>(viewport.y),
          },
      .extent =
          {
              static_cast<u32>(viewport.width),
              static_cast<u32>(viewport.height),
          },
  };

  vkCmdSetScissor(command_buffer_handle, 0, 1, &scissor);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet