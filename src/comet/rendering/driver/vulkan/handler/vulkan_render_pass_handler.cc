// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_render_pass_handler.h"

#include "comet/core/logger.h"
#include "comet/core/memory/memory.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
RenderPassHandler::RenderPassHandler(const RenderPassHandlerDescr& descr)
    : Handler{descr}, swapchain_{descr.swapchain} {
  COMET_ASSERT(swapchain_ != nullptr, "Swapchain is null!");
}

void RenderPassHandler::Shutdown() {
  for (auto& pair : render_passes_) {
    Destroy(pair.second, true);
  }

  render_passes_.clear();
  Handler::Shutdown();
}

RenderPass* RenderPassHandler::Generate(const RenderPassDescr& descr) {
  auto& device{context_->GetDevice()};

  RenderPass render_pass;
  render_pass.id = descr.id;
  render_pass.clear_flags = descr.clear_flags;
  render_pass.extent = descr.extent;
  render_pass.offset = descr.offset;
  memory::CopyMemory(render_pass.clear_values.data(), descr.clear_values.data(),
                     sizeof(descr.clear_values[0]) * descr.clear_values.size());

  auto is_msaa{device.IsMsaa()};
  const auto msaa_samples{is_msaa ? device.GetMsaaSamples()
                                  : VK_SAMPLE_COUNT_1_BIT};
  std::vector<VkAttachmentDescription> color_attachment_descrs{};
  std::vector<VkAttachmentDescription> depth_attachment_descrs{};
  std::vector<VkAttachmentDescription> resolve_attachment_descrs{};

  for (const auto& attachment_descr : descr.attachment_descrs) {
    VkAttachmentDescription vk_descr{};

    switch (attachment_descr.type) {
      case AttachmentType::Color: {
        auto is_clear{
            (render_pass.clear_flags & RenderPassClearFlag::ColorBuffer) != 0};

        vk_descr.format = attachment_descr.format == VK_FORMAT_UNDEFINED
                              ? swapchain_->GetFormat()
                              : attachment_descr.format;

        auto are_samples_empty{
            (attachment_descr.samples & VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM) ==
            VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM};

        COMET_ASSERT(
            are_samples_empty || is_msaa ||
                (attachment_descr.samples & VK_SAMPLE_COUNT_1_BIT) ==
                    VK_SAMPLE_COUNT_1_BIT,
            "Samples provided are more than 1 bit, but MSAA is disabled!");

        vk_descr.samples =
            are_samples_empty ? msaa_samples : attachment_descr.samples;

        if (attachment_descr.load_op == VK_ATTACHMENT_LOAD_OP_DONT_CARE) {
          vk_descr.loadOp =
              is_clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : attachment_descr.load_op;
        } else {
          vk_descr.loadOp = attachment_descr.load_op;
        }

        vk_descr.storeOp = attachment_descr.store_op;

        // Not used on color attachments.
        vk_descr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        vk_descr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // If coming from another render pass,
        // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL is needed.
        vk_descr.initialLayout = vk_descr.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD
                                     ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                                     : VK_IMAGE_LAYOUT_UNDEFINED;

        vk_descr.finalLayout = attachment_descr.is_final_layout
                                   ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                                   : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        vk_descr.flags = 0;
        color_attachment_descrs.push_back(vk_descr);
        break;
      }
      case AttachmentType::Depth: {
        auto is_clear{
            (render_pass.clear_flags & RenderPassClearFlag::DepthBuffer) != 0};

        vk_descr.format = attachment_descr.format == VK_FORMAT_UNDEFINED
                              ? device.ChooseDepthFormat()
                              : attachment_descr.format;

        vk_descr.samples = msaa_samples;

        if (attachment_descr.load_op == VK_ATTACHMENT_LOAD_OP_DONT_CARE) {
          vk_descr.loadOp =
              is_clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : attachment_descr.load_op;
        } else {
          vk_descr.loadOp = attachment_descr.load_op;
        }

        vk_descr.storeOp = attachment_descr.store_op;

        // TODO(m4jr0): Handle stencil attachments.
        vk_descr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        vk_descr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // If coming from another render pass,
        // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL is needed.
        vk_descr.initialLayout =
            vk_descr.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD
                ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                : VK_IMAGE_LAYOUT_UNDEFINED;

        vk_descr.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        vk_descr.flags = 0;
        depth_attachment_descrs.push_back(vk_descr);
        break;
      }
      case AttachmentType::Resolve: {
        vk_descr.format = attachment_descr.format == VK_FORMAT_UNDEFINED
                              ? swapchain_->GetFormat()
                              : attachment_descr.format;
        vk_descr.samples = VK_SAMPLE_COUNT_1_BIT;
        vk_descr.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        vk_descr.storeOp = attachment_descr.store_op;
        vk_descr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        vk_descr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        vk_descr.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vk_descr.finalLayout = attachment_descr.is_final_layout
                                   ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                                   : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        resolve_attachment_descrs.push_back(vk_descr);
        break;
      }
      default: {
        COMET_LOG_RENDERING_ERROR(
            "Unsupported render pass attachment type: ",
            static_cast<std::underlying_type_t<AttachmentType>>(
                attachment_descr.type),
            "! Ignoring.");
        continue;
      }
    }
  }

  const auto color_attachment_count{
      static_cast<u32>(color_attachment_descrs.size())};
  const auto depth_attachment_count{
      static_cast<u32>(depth_attachment_descrs.size())};
  const auto resolve_attachment_count{
      static_cast<u32>(resolve_attachment_descrs.size())};

  COMET_ASSERT(depth_attachment_count <= 1,
               "Several depth attachments were provided, but with only one "
               "subpass, the others would be ignored!");

  std::vector<VkAttachmentDescription> attachment_descrs{};
  attachment_descrs.reserve(static_cast<usize>(color_attachment_count) +
                            depth_attachment_count + resolve_attachment_count);

  for (const auto& vk_descr : color_attachment_descrs) {
    attachment_descrs.push_back(vk_descr);
  }

  for (const auto& vk_descr : depth_attachment_descrs) {
    attachment_descrs.push_back(vk_descr);
  }

  for (const auto& vk_descr : resolve_attachment_descrs) {
    attachment_descrs.push_back(vk_descr);
  }

  const auto image_count{swapchain_->GetImageCount()};
  render_pass.render_targets.resize(image_count);
  const auto& swapchain_images{swapchain_->GetImages()};

  for (u32 i{0}; i < image_count; ++i) {
    auto& render_target{render_pass.render_targets[i]};
    render_target.attachments.reserve(attachment_descrs.size());
    auto swapchain_image_view{swapchain_images[i].image_view_handle};

    for (const auto& vk_descr : color_attachment_descrs) {
      render_target.attachments.push_back(
          Attachment{AttachmentType::Color, vk_descr,
                     is_msaa ? swapchain_->GetColorImage().image_view_handle
                             : swapchain_image_view});
    }

    for (const auto& vk_descr : depth_attachment_descrs) {
      render_target.attachments.push_back(
          Attachment{AttachmentType::Depth, vk_descr,
                     swapchain_->GetDepthImage().image_view_handle});
    }

    for (const auto& vk_descr : resolve_attachment_descrs) {
      render_target.attachments.push_back(
          Attachment{AttachmentType::Resolve, vk_descr, swapchain_image_view});
    }
  }

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  u32 attachment_index{0};

  subpass.colorAttachmentCount = color_attachment_count;
  std::vector<VkAttachmentReference> color_attachment_refs{};
  color_attachment_refs.reserve(color_attachment_count);

  if (color_attachment_count == 0) {
    subpass.pColorAttachments = VK_NULL_HANDLE;
  } else {
    for (u32 i{0}; i < color_attachment_count; ++i) {
      color_attachment_refs.push_back(
          {attachment_index++, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    }

    subpass.pColorAttachments = color_attachment_refs.data();
  }

  VkAttachmentReference depth_attachment_ref{};

  if (depth_attachment_count == 0) {
    subpass.pDepthStencilAttachment = VK_NULL_HANDLE;
  } else {
    depth_attachment_ref.attachment = attachment_index++;
    depth_attachment_ref.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;
  }

  std::vector<VkAttachmentReference> resolve_attachment_refs{};
  resolve_attachment_refs.reserve(resolve_attachment_count);

  if (resolve_attachment_count == 0) {
    subpass.pResolveAttachments = VK_NULL_HANDLE;
  } else {
    for (u32 i{0}; i < resolve_attachment_count; ++i) {
      resolve_attachment_refs.push_back(
          {attachment_index++, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    }

    subpass.pResolveAttachments = resolve_attachment_refs.data();
  }

  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = VK_NULL_HANDLE;

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = static_cast<u32>(attachment_descrs.size());
  render_pass_info.pAttachments = attachment_descrs.data();
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &descr.dependency;
  render_pass_info.pNext = VK_NULL_HANDLE;
  render_pass_info.flags = 0;

  COMET_CHECK_VK(vkCreateRenderPass(device, &render_pass_info, VK_NULL_HANDLE,
                                    &render_pass.handle),
                 "Failed to create render pass!");

  GenerateFrameBuffers(render_pass);

  auto insert_pair{render_passes_.emplace(render_pass.id, render_pass)};
  COMET_ASSERT(insert_pair.second, "Could not insert render pass: ",
               COMET_STRING_ID_LABEL(render_pass.id), "!");
  return &insert_pair.first->second;
}

RenderPass* RenderPassHandler::Get(RenderPassId render_pass_id) {
  auto* render_pass{TryGet(render_pass_id)};
  COMET_ASSERT(render_pass != nullptr,
               "Requested render_pass does not exist: ", render_pass_id, "!");
  return render_pass;
}

RenderPass* RenderPassHandler::TryGet(RenderPassId render_pass_id) {
  auto it{render_passes_.find(render_pass_id)};

  if (it == render_passes_.end()) {
    return nullptr;
  }

  return &it->second;
}

RenderPass* RenderPassHandler::GetOrGenerate(const RenderPassDescr& descr) {
  auto* render_pass{TryGet(descr.id)};

  if (render_pass != nullptr) {
    return render_pass;
  }

  return Generate(descr);
}

void RenderPassHandler::Destroy(RenderPassId render_pass_id) {
  Destroy(*Get(render_pass_id));
}

void RenderPassHandler::Destroy(RenderPass& render_pass) {
  return Destroy(render_pass, false);
}

void RenderPassHandler::BeginPass(const RenderPass& pass,
                                  VkCommandBuffer command_buffer_handle,
                                  ImageIndex image_index) const {
  COMET_ASSERT(image_index < pass.render_targets.size(),
               "Image index is too high!");
  auto render_pass_begin_info{init::GenerateRenderPassBeginInfo(
      pass.handle, pass.extent,
      pass.render_targets[image_index].framebuffer_handle)};

  render_pass_begin_info.clearValueCount =
      static_cast<u32>(pass.clear_values.size());
  render_pass_begin_info.pClearValues = pass.clear_values.data();
  render_pass_begin_info.renderArea.offset = pass.offset;

  vkCmdBeginRenderPass(command_buffer_handle, &render_pass_begin_info,
                       VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPassHandler::BeginPass(RenderPassId render_pass_id,
                                  VkCommandBuffer command_buffer_handle,
                                  ImageIndex image_index) const {
  auto it{render_passes_.find(render_pass_id)};
  COMET_ASSERT(it != render_passes_.end(),
               "Unable to find render pass with ID: ",
               COMET_STRING_ID_LABEL(render_pass_id), "!");
  BeginPass(it->second, command_buffer_handle, image_index);
}

void RenderPassHandler::EndPass(VkCommandBuffer command_buffer_handle) const {
  vkCmdEndRenderPass(command_buffer_handle);
}

void RenderPassHandler::Refresh(RenderPassId render_pass_id) {
  Refresh(*Get(render_pass_id));
}

void RenderPassHandler::Refresh(RenderPass& render_pass) {
  DestroyFrameBuffers(render_pass);
  const auto& swapchain_images{swapchain_->GetImages()};
  const auto image_count{swapchain_->GetImageCount()};
  const auto is_msaa{context_->GetDevice().IsMsaa()};

  for (u32 i{0}; i < image_count; ++i) {
    auto& render_target{render_pass.render_targets[i]};
    auto swapchain_image_view{swapchain_images[i].image_view_handle};

    for (auto& attachment : render_target.attachments) {
      if (attachment.type == AttachmentType::Color) {
        attachment.image_view_handle =
            is_msaa ? swapchain_->GetColorImage().image_view_handle
                    : swapchain_image_view;
        continue;
      }

      if (attachment.type == AttachmentType::Depth) {
        attachment.image_view_handle =
            swapchain_->GetDepthImage().image_view_handle;
        continue;
      }

      if (attachment.type == AttachmentType::Resolve) {
        attachment.image_view_handle = swapchain_image_view;
        continue;
      }

      COMET_ASSERT(
          false, "Unknown or unsupported attachment type: ",
          static_cast<std::underlying_type_t<AttachmentType>>(attachment.type),
          "!");
    }
  }

  GenerateFrameBuffers(render_pass);
}

void RenderPassHandler::Destroy(RenderPass& render_pass,
                                bool is_destroying_handler) {
  if (render_pass.handle != VK_NULL_HANDLE) {
    vkDestroyRenderPass(context_->GetDevice(), render_pass.handle,
                        VK_NULL_HANDLE);
    render_pass.handle = VK_NULL_HANDLE;
  }

  DestroyFrameBuffers(render_pass);

  if (!is_destroying_handler) {
    render_passes_.erase(render_pass.id);
  }

  render_pass.clear_flags = RenderPassClearFlag::None;
  render_pass.extent = {};
  render_pass.offset = {};
  render_pass.id = kInvalidRenderPassId;
  render_pass.render_targets.clear();
  render_pass.clear_values = {};
}

void RenderPassHandler::GenerateFrameBuffers(RenderPass& render_pass) const {
  auto create_info{init::GenerateFrameBufferCreateInfo(render_pass.handle,
                                                       render_pass.extent)};

  for (auto& render_target : render_pass.render_targets) {
    VkImageView image_view_handles[32]{VK_NULL_HANDLE};
    auto attachment_count{render_target.attachments.size()};

    for (u32 i{0}; i < attachment_count; ++i) {
      image_view_handles[i] = render_target.attachments[i].image_view_handle;
    }

    create_info.attachmentCount = static_cast<u32>(attachment_count);
    create_info.pAttachments = image_view_handles;

    COMET_CHECK_VK(
        vkCreateFramebuffer(context_->GetDevice(), &create_info, VK_NULL_HANDLE,
                            &render_target.framebuffer_handle),
        "Failed to create framebuffer!");
  }
}

void RenderPassHandler::DestroyFrameBuffers(RenderPass& render_pass) const {
  for (auto& render_target : render_pass.render_targets) {
    vkDestroyFramebuffer(context_->GetDevice(),
                         render_target.framebuffer_handle, VK_NULL_HANDLE);
    render_target.framebuffer_handle = VK_NULL_HANDLE;
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
