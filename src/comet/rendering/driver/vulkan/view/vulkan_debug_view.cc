// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

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

  render_pass_descr.dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  render_pass_descr.dependency.dstSubpass = 0;
  // Store op is always performed in late tests, after subpass access.
  render_pass_descr.dependency.srcStageMask =
      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  // Load op is always performed in early tests, before subpass access.
  render_pass_descr.dependency.dstStageMask =
      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  render_pass_descr.dependency.srcAccessMask =
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
  render_pass_descr.dependency.dstAccessMask =
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
  render_pass_descr.dependency.dependencyFlags = 0;

  // TODO(m4jr0): Make clear values more configurable.
  memory::CopyMemory(
      &render_pass_descr.clear_values[0].color, clear_color_,
      sizeof(render_pass_descr.clear_values[0].color.float32[0]) * 4);
  render_pass_descr.clear_values[1].depthStencil.depth = 1.0f;

  auto is_msaa{context_->GetDevice().IsMsaa()};
  is_msaa = false;
  render_pass_descr.clear_flags =
      RenderPassClearFlag::ColorBuffer | RenderPassClearFlag::DepthBuffer;
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

void DebugView::Update(frame::FramePacket*) {
  COMET_PROFILE("DebugView::Update");
  // TODO(m4jr0): Display 3D debug information.
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet