// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_world_view.h"

#include "comet/core/memory/memory_utils.h"

namespace comet {
namespace rendering {
namespace vk {
WorldView::WorldView(const WorldViewDescr& descr)
    : ShaderView{descr}, render_proxy_handler_{descr.render_proxy_handler} {
  COMET_ASSERT(render_proxy_handler_ != nullptr,
               "Render proxy handler is null!");
}

void WorldView::Initialize() {
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
  render_pass_descr.dependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  render_pass_descr.dependency.srcAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  render_pass_descr.dependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  render_pass_descr.dependency.dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  render_pass_descr.dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // TODO(m4jr0): Make clear values more configurable.
  memory::CopyMemory(
      &render_pass_descr.clear_values[0].color, clear_color_,
      sizeof(render_pass_descr.clear_values[0].color.float32[0]) * 4);
  render_pass_descr.clear_values[1].depthStencil.depth = 1.0f;

  const auto is_msaa{context_->GetDevice().IsMsaa()};
  render_pass_descr.clear_flags =
      RenderPassClearFlag::ColorBuffer | RenderPassClearFlag::DepthBuffer;
  render_pass_descr.attachment_descrs.reserve(3);

  AttachmentDescr color_attachment_descr{};
  color_attachment_descr.type = AttachmentType::Color;
  color_attachment_descr.load_op =
      is_first_ ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
  color_attachment_descr.store_op = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment_descr.is_final_layout = !is_msaa && is_last_;
  render_pass_descr.attachment_descrs.push_back(color_attachment_descr);

  AttachmentDescr depth_attachment_descr{};
  depth_attachment_descr.type = AttachmentType::Depth;
  depth_attachment_descr.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment_descr.store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment_descr.is_final_layout = false;
  render_pass_descr.attachment_descrs.push_back(depth_attachment_descr);

  if (is_msaa) {
    AttachmentDescr resolve_attachment_descr{};
    resolve_attachment_descr.type = AttachmentType::Resolve;
    resolve_attachment_descr.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
    resolve_attachment_descr.store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    resolve_attachment_descr.is_final_layout = is_first_;
    render_pass_descr.attachment_descrs.push_back(resolve_attachment_descr);
  }

  render_pass_ = render_pass_handler_->Generate(render_pass_descr);

  ShaderDescr shader_descr{};
  shader_descr.resource_path =
      COMET_TCHAR("shaders/vulkan/default_shader.vk.cshader");
  shader_descr.render_pass = render_pass_;
  shader_ = shader_handler_->Generate(shader_descr);
}

void WorldView::Destroy() {
  shader_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
  View::Destroy();
}

void WorldView::Update(const ViewPacket& packet) {
  render_proxy_handler_->Update();
  render_pass_handler_->BeginPass(*render_pass_, packet.command_buffer_handle,
                                  packet.image_index);
  shader_handler_->Bind(*shader_);
  ShaderPacket shader_packet{};
  shader_packet.projection_matrix = &packet.projection_matrix;
  shader_packet.view_matrix = packet.view_matrix;
  shader_handler_->UpdateGlobal(*shader_, shader_packet);
  render_proxy_handler_->DrawProxies(*shader_);
  render_pass_handler_->EndPass(packet.command_buffer_handle);
  shader_handler_->Reset();
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet