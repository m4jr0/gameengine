// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_debug_view.h"

#include "comet/core/memory/memory_utils.h"
#include "comet/profiler/profiler.h"

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
  render_pass_descr.id = id_;

  VkExtent2D extent{};
  extent.width = width_;
  extent.height = height_;

  render_pass_descr.extent = extent;
  render_pass_descr.offset.x = 0;
  render_pass_descr.offset.y = 0;

  render_pass_descr.dependencies = frame::FrameArray<VkSubpassDependency>{};
  render_pass_descr.dependencies.Reserve(1);

  auto& dependency{render_pass_descr.dependencies.EmplaceBack()};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  // Store op is always performed in late tests, after subpass access.
  dependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  // Load op is always performed in early tests, before subpass access.
  dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
  dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
  dependency.dependencyFlags = 0;

  // TODO(m4jr0): Make clear values more configurable.
  memory::CopyMemory(
      &render_pass_descr.clear_values[0].color, clear_color_,
      sizeof(render_pass_descr.clear_values[0].color.float32[0]) * 4);
  render_pass_descr.clear_values[1].depthStencil.depth = 1.0f;

  auto is_msaa{context_->GetDevice().IsMsaa()};
  is_msaa = false;
  render_pass_descr.clear_flags =
      RenderPassClearFlag::ColorBuffer | RenderPassClearFlag::DepthBuffer;
  render_pass_descr.attachment_descrs = frame::FrameArray<AttachmentDescr>{};
  render_pass_descr.attachment_descrs.Reserve(3);

  AttachmentDescr color_attachment_descr{};
  color_attachment_descr.type = AttachmentType::Color;
  color_attachment_descr.load_op =
      is_first_ ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
  color_attachment_descr.store_op = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment_descr.is_final_layout = !is_msaa && is_last_;
  render_pass_descr.attachment_descrs.PushBack(color_attachment_descr);

  AttachmentDescr depth_attachment_descr{};
  depth_attachment_descr.type = AttachmentType::Depth;
  depth_attachment_descr.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment_descr.store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment_descr.is_final_layout = false;
  render_pass_descr.attachment_descrs.PushBack(depth_attachment_descr);

  if (is_msaa) {
    AttachmentDescr resolve_attachment_descr{};
    resolve_attachment_descr.type = AttachmentType::Resolve;
    resolve_attachment_descr.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
    resolve_attachment_descr.store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    resolve_attachment_descr.is_final_layout = is_first_;
    render_pass_descr.attachment_descrs.PushBack(resolve_attachment_descr);
  }

  render_pass_ = render_pass_handler_->Generate(render_pass_descr);

  ShaderDescr shader_descr{};
  shader_descr.resource_path =
      COMET_TCHAR("shaders/vulkan/debug_shader.vk.cshader");
  shader_descr.render_pass = render_pass_;
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
  if (packet->is_rendering_skipped) {
    return;
  }

  shader_handler_->UpdateGlobals(shader_, packet);
  shader_handler_->UpdateStorages(shader_, packet);

  auto draw_count{render_proxy_handler_->GetRenderProxyCount()};
  shader_handler_->UpdateConstants(shader_, {nullptr, &draw_count});
  render_proxy_handler_->DebugCull(shader_);

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  render_pass_handler_->BeginPass(render_pass_, command_buffer_handle,
                                  context_->GetImageIndex());
  render_proxy_handler_->DrawDebugCull(shader_);
  render_pass_handler_->EndPass(command_buffer_handle);
#endif  // COMET_DEBUG_CULLING
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet