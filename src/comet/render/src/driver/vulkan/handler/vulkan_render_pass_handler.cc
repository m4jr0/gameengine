// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/handler/vulkan_render_pass_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/hash/hash.h"
#include "comet/core/logger/logging.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/container/array.h"
#include "comet/core/type_trait.h"
#include "comet/core/math/math_scalar.h"
#include "comet/render/driver/vulkan/label/vulkan_render_pass_label.h"
#include "comet/render/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/render/driver/vulkan/utils/vulkan_render_pass_utils.h"
#include "comet/render/driver/vulkan/vulkan_context.h"
#include "comet/render/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace render {
namespace vk {
RenderPassHandler::RenderPassHandler(const RenderPassHandlerDescr& descr)
    : Handler{descr},
      render_passes_{&cache_allocator_, 32},
      swapchain_{descr.swapchain} {
  COMET_ASSERT(swapchain_ != nullptr, "RenderPassHandler::RenderPassHandler",
               "swapchain is null");
}

RenderPassHandle RenderPassHandler::GetOrGenerate(
    const RenderPassDescr& descr) {
  const auto hash{GenerateHash(descr)};

  if (const auto handle{render_passes_.TryAcquire(hash)}; handle) {
    return handle;
  }

  auto* render_pass{GenerateRenderPass(descr)};
  COMET_ASSERT(render_pass != nullptr, "RenderPassHandler::GetOrGenerate",
               "generated render pass is null");

  const auto handle{render_passes_.Create(hash, render_pass)};
  COMET_ASSERT(handle, "RenderPassHandler::GetOrGenerate",
               "render pass instance creation failed");

  render_pass->handle = handle;
  return handle;
}

void RenderPassHandler::Destroy(RenderPassHandle handle) {
  auto* render_pass{render_passes_.Get(handle)};
  COMET_ASSERT(render_pass->handle == handle, "RenderPassHandler::Destroy",
               "render pass handle mismatch", "expected_handle", handle,
               "actual_handle", render_pass->handle);

  if (!render_passes_.Release(handle)) {
    return;
  }

  DestroyRenderPass(render_pass);
  render_passes_.Remove(handle);
}

void RenderPassHandler::BeginPass(RenderPassHandle handle, VkCommandBuffer cmd,
                                  ImageIndex image_index,
                                  const VkClearValue* clear_values,
                                  u32 clear_value_count) const {
  COMET_ASSERT(cmd != VK_NULL_HANDLE, "RenderPassHandler::BeginPass",
               "command buffer is invalid");

  const auto* render_pass{Get(handle)};
  COMET_ASSERT(image_index < render_pass->render_targets.GetSize(),
               "RenderPassHandler::BeginPass", "image index is out of bounds",
               "image_index", image_index, "render_target_count",
               render_pass->render_targets.GetSize());

  VkRenderPassBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  begin_info.pNext = nullptr;
  begin_info.renderPass = render_pass->vk_handle;
  begin_info.framebuffer =
      render_pass->render_targets[image_index].framebuffer_handle;
  begin_info.renderArea.offset = render_pass->offset;
  begin_info.renderArea.extent = render_pass->extent;
  begin_info.clearValueCount = clear_value_count;
  begin_info.pClearValues = clear_values;

  vkCmdBeginRenderPass(cmd, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPassHandler::BeginPass(RenderPassHandle handle, VkCommandBuffer cmd,
                                  VkFramebuffer framebuffer,
                                  const VkClearValue* clear_values,
                                  u32 clear_value_count) const {
  COMET_ASSERT(cmd != VK_NULL_HANDLE, "RenderPassHandler::BeginPass",
               "command buffer is invalid");
  COMET_ASSERT(framebuffer != VK_NULL_HANDLE, "RenderPassHandler::BeginPass",
               "framebuffer is invalid");

  const auto* render_pass{Get(handle)};

  VkRenderPassBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  begin_info.pNext = nullptr;
  begin_info.renderPass = render_pass->vk_handle;
  begin_info.framebuffer = framebuffer;
  begin_info.renderArea.offset = render_pass->offset;
  begin_info.renderArea.extent = render_pass->extent;
  begin_info.clearValueCount = clear_value_count;
  begin_info.pClearValues = clear_values;

  vkCmdBeginRenderPass(cmd, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPassHandler::EndPass(VkCommandBuffer cmd) const {
  vkCmdEndRenderPass(cmd);
}

void RenderPassHandler::SetSize(RenderPassHandle handle, u32 width,
                                u32 height) {
  auto* render_pass{Get(handle)};
  render_pass->extent = {.width = width, .height = height};
  Refresh(*render_pass);
}

void RenderPassHandler::Refresh(RenderPassHandle handle) {
  auto* render_pass{Get(handle)};
  Refresh(*render_pass);
}

VkRenderPass RenderPassHandler::GetVkHandle(RenderPassHandle handle) const {
  return Get(handle)->vk_handle;
}

VkSampleCountFlagBits RenderPassHandler::GetSamples(
    RenderPassHandle handle) const {
  return Get(handle)->samples;
}

VkExtent2D RenderPassHandler::GetExtent(RenderPassHandle handle) const {
  return Get(handle)->extent;
}

RenderPass* RenderPassHandler::Get(RenderPassHandle handle) {
  auto* render_pass{render_passes_.TryGet(handle)};
  COMET_ASSERT(render_pass != nullptr, "RenderPassHandler::Get",
               "render pass not found", "render_pass_handle", handle);
  return render_pass;
}

const RenderPass* RenderPassHandler::Get(RenderPassHandle handle) const {
  const auto* render_pass{render_passes_.TryGet(handle)};
  COMET_ASSERT(render_pass != nullptr, "RenderPassHandler::Get",
               "render pass not found", "render_pass_handle", handle);
  return render_pass;
}

void RenderPassHandler::OnInitialize() {
  allocator_.Initialize();
  render_passes_.Initialize();
}

void RenderPassHandler::OnShutdown() {
  memory::PlatformAllocator tmp_allocator{kEngineMemoryTagRender};
  Array<RenderPassHandle> handles_to_destroy{&tmp_allocator};

  render_passes_.ForEachLive(
      [&handles_to_destroy](RenderPassHandle handle, const RenderPass*) {
        handles_to_destroy.PushLast(handle);
      });

  for (const auto handle : handles_to_destroy) {
    const auto ref_count{render_passes_.GetRefCount(handle)};

    if (ref_count > 0) {
      COMET_LOG_WARNING(LoggerType::Rendering, "RenderPassHandler::OnShutdown",
                        "forcing render pass destruction", "render_pass_handle",
                        handle, "ref_count", ref_count);
    }

    auto* render_pass{render_passes_.Drain(handle)};

    if (render_pass == nullptr) {
      continue;
    }

    COMET_ASSERT(render_pass->handle == handle, "RenderPassHandler::OnShutdown",
                 "render pass handle mismatch", "expected_handle", handle,
                 "actual_handle", render_pass->handle);

    DestroyRenderPass(render_pass);
  }

  render_passes_.Destroy();
  allocator_.Destroy();
}

RenderPass* RenderPassHandler::GenerateRenderPass(
    const RenderPassDescr& descr) {
  auto& device{context_->GetDevice()};

  auto* render_pass{allocator_.AllocateOneAndPopulate<RenderPass>()};
  render_pass->handle = RenderPassHandle::Invalid();
  render_pass->clear_flags = descr.clear_flags;
  render_pass->extent = descr.extent;
  render_pass->offset = descr.offset;

  const auto is_msaa{IsMultisampled(descr.options) && device.IsMsaa()};
  const auto msaa_samples{is_msaa ? device.GetMsaaSamples()
                                  : VK_SAMPLE_COUNT_1_BIT};

  frame::FrameArray<VkAttachmentDescription> color_attachment_descrs{};
  frame::FrameArray<VkAttachmentDescription> depth_attachment_descrs{};
  frame::FrameArray<VkAttachmentDescription> resolve_attachment_descrs{};

  for (const auto& attachment_descr : descr.attachment_descrs) {
    VkAttachmentDescription vk_descr{};

    switch (attachment_descr.type) {
      case AttachmentType::Color: {
        const auto is_clear{(render_pass->clear_flags &
                             kRenderPassClearFlagBitsColorBuffer) != 0};

        vk_descr.format = attachment_descr.format == VK_FORMAT_UNDEFINED
                              ? swapchain_->GetFormat()
                              : attachment_descr.format;

        const auto are_samples_empty{
            (attachment_descr.samples & VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM) ==
            VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM};

        COMET_ASSERT(are_samples_empty || is_msaa ||
                         (attachment_descr.samples & VK_SAMPLE_COUNT_1_BIT) ==
                             VK_SAMPLE_COUNT_1_BIT,
                     "RenderPassHandler::GenerateRenderPass",
                     "attachment sample count is invalid when msaa is disabled",
                     "samples_value", ToUnderlying(attachment_descr.samples));

        vk_descr.samples =
            are_samples_empty ? msaa_samples : attachment_descr.samples;

        if (attachment_descr.load_op == VK_ATTACHMENT_LOAD_OP_DONT_CARE) {
          vk_descr.loadOp =
              is_clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : attachment_descr.load_op;
        } else {
          vk_descr.loadOp = attachment_descr.load_op;
        }

        vk_descr.storeOp = attachment_descr.store_op;
        vk_descr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        vk_descr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        vk_descr.initialLayout = vk_descr.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD
                                     ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                                     : VK_IMAGE_LAYOUT_UNDEFINED;
        vk_descr.finalLayout = attachment_descr.is_final_layout
                                   ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                                   : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        vk_descr.flags = 0;

        color_attachment_descrs.PushLast(vk_descr);
        break;
      }

      case AttachmentType::Depth: {
        const auto is_clear{(render_pass->clear_flags &
                             kRenderPassClearFlagBitsDepthBuffer) != 0};

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
        vk_descr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        vk_descr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        vk_descr.initialLayout =
            vk_descr.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD
                ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                : VK_IMAGE_LAYOUT_UNDEFINED;
        vk_descr.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        vk_descr.flags = 0;

        depth_attachment_descrs.PushLast(vk_descr);
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
        vk_descr.flags = 0;

        resolve_attachment_descrs.PushLast(vk_descr);
        break;
      }

      default: {
        COMET_ASSERT(false, "RenderPassHandler::GenerateRenderPass",
                     "attachment type is unsupported", "attachment_type",
                     GetAttachmentTypeLabel(attachment_descr.type),
                     "attachment_type_value",
                     ToUnderlying(attachment_descr.type));
        continue;
      }
    }
  }

  render_pass->samples = VK_SAMPLE_COUNT_1_BIT;

  if (!color_attachment_descrs.IsEmpty()) {
    render_pass->samples = color_attachment_descrs[0].samples;
  } else if (!depth_attachment_descrs.IsEmpty()) {
    render_pass->samples = depth_attachment_descrs[0].samples;
  }

  const auto color_attachment_count{
      static_cast<u32>(color_attachment_descrs.GetSize())};
  const auto depth_attachment_count{
      static_cast<u32>(depth_attachment_descrs.GetSize())};
  const auto resolve_attachment_count{
      static_cast<u32>(resolve_attachment_descrs.GetSize())};

  COMET_ASSERT(depth_attachment_count <= 1,
               "RenderPassHandler::GenerateRenderPass",
               "depth attachment count exceeds supported limit",
               "depth_attachment_count", depth_attachment_count);

  frame::FrameArray<VkAttachmentDescription> attachment_descrs{};
  attachment_descrs.Reserve(static_cast<usize>(color_attachment_count) +
                            depth_attachment_count + resolve_attachment_count);

  for (const auto& vk_descr : color_attachment_descrs) {
    attachment_descrs.PushLast(vk_descr);
  }

  for (const auto& vk_descr : depth_attachment_descrs) {
    attachment_descrs.PushLast(vk_descr);
  }

  for (const auto& vk_descr : resolve_attachment_descrs) {
    attachment_descrs.PushLast(vk_descr);
  }

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  u32 attachment_index{0};

  subpass.colorAttachmentCount = color_attachment_count;
  frame::FrameArray<VkAttachmentReference> color_attachment_refs{};
  color_attachment_refs.Reserve(color_attachment_count);

  if (color_attachment_count == 0) {
    subpass.pColorAttachments = nullptr;
  } else {
    for (u32 i{0}; i < color_attachment_count; ++i) {
      color_attachment_refs.EmplaceLast(
          attachment_index++, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    subpass.pColorAttachments = color_attachment_refs.GetData();
  }

  VkAttachmentReference depth_attachment_ref{};

  if (depth_attachment_count == 0) {
    subpass.pDepthStencilAttachment = nullptr;
  } else {
    depth_attachment_ref.attachment = attachment_index++;
    depth_attachment_ref.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;
  }

  frame::FrameArray<VkAttachmentReference> resolve_attachment_refs{};
  resolve_attachment_refs.Reserve(resolve_attachment_count);

  if (resolve_attachment_count == 0) {
    subpass.pResolveAttachments = nullptr;
  } else {
    for (u32 i{0}; i < resolve_attachment_count; ++i) {
      resolve_attachment_refs.EmplaceLast(
          attachment_index++, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    subpass.pResolveAttachments = resolve_attachment_refs.GetData();
  }

  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = nullptr;

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.pNext = nullptr;
  render_pass_info.flags = 0;
  render_pass_info.attachmentCount =
      static_cast<u32>(attachment_descrs.GetSize());
  render_pass_info.pAttachments = attachment_descrs.GetData();
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount =
      static_cast<u32>(descr.dependencies.GetSize());
  render_pass_info.pDependencies = descr.dependencies.GetData();

  COMET_CHECK_VK(vkCreateRenderPass(device, &render_pass_info, nullptr,
                                    &render_pass->vk_handle),
                 "RenderPassHandler::GenerateRenderPass",
                 "render pass creation failed");

  COMET_VK_SET_DEBUG_LABEL(render_pass->vk_handle, "render_pass");

  if (IsSwapchainTarget(descr.options)) {
    const auto image_count{swapchain_->GetImageCount()};
    const auto& swapchain_images{swapchain_->GetImages()};

    render_pass->render_targets = Array<VulkanRenderTarget>{&allocator_};
    render_pass->render_targets.Resize(image_count);

    for (u32 i{0}; i < image_count; ++i) {
      auto& render_target{render_pass->render_targets[i]};
      render_target.attachments = Array<Attachment>{&allocator_};
      render_target.attachments.Reserve(attachment_descrs.GetSize());

      const auto swapchain_image_view{swapchain_images[i].image_view_handle};

      for (const auto& vk_descr : color_attachment_descrs) {
        render_target.attachments.PushLast(
            Attachment{AttachmentType::Color, vk_descr,
                       is_msaa ? swapchain_->GetColorImage().image_view_handle
                               : swapchain_image_view});
      }

      for (const auto& vk_descr : depth_attachment_descrs) {
        render_target.attachments.PushLast(
            Attachment{AttachmentType::Depth, vk_descr,
                       swapchain_->GetDepthImage().image_view_handle});
      }

      for (const auto& vk_descr : resolve_attachment_descrs) {
        render_target.attachments.PushLast(Attachment{
            AttachmentType::Resolve, vk_descr, swapchain_image_view});
      }
    }

    GenerateFrameBuffers(*render_pass);
  }

  return render_pass;
}

void RenderPassHandler::DestroyRenderPass(RenderPass* render_pass) {
  COMET_ASSERT(render_pass != nullptr, "RenderPassHandler::DestroyRenderPass",
               "render pass is null");

  DestroyFrameBuffers(*render_pass);

  if (render_pass->vk_handle != VK_NULL_HANDLE) {
    vkDestroyRenderPass(context_->GetDevice(), render_pass->vk_handle, nullptr);
    render_pass->vk_handle = VK_NULL_HANDLE;
  }

  for (auto& render_target : render_pass->render_targets) {
    render_target.attachments.Release();
  }

  render_pass->render_targets.Release();
  render_pass->handle.Invalidate();
  allocator_.Deallocate(render_pass);
}

void RenderPassHandler::GenerateFrameBuffers(RenderPass& render_pass) const {
  auto create_info{init::GenerateFrameBufferCreateInfo(render_pass.vk_handle,
                                                       render_pass.extent)};

  for (auto& render_target : render_pass.render_targets) {
    constexpr usize kMaxAttachmentCount{32};
    VkImageView image_view_handles[kMaxAttachmentCount]{VK_NULL_HANDLE};

    const auto attachment_count{render_target.attachments.GetSize()};

    COMET_ASSERT(attachment_count <= kMaxAttachmentCount,
                 "RenderPassHandler::GenerateFrameBuffers",
                 "attachment count exceeds local capacity", "attachment_count",
                 attachment_count, "max_attachment_count", kMaxAttachmentCount);

    const auto resolved_attachment_count{
        math::Min(attachment_count, kMaxAttachmentCount)};

    for (usize i{0}; i < resolved_attachment_count; ++i) {
      image_view_handles[i] = render_target.attachments[i].image_view_handle;
    }

    create_info.attachmentCount = static_cast<u32>(attachment_count);
    create_info.pAttachments = image_view_handles;

    COMET_CHECK_VK(
        vkCreateFramebuffer(context_->GetDevice(), &create_info, nullptr,
                            &render_target.framebuffer_handle),
        "RenderPassHandler::GenerateFrameBuffers",
        "framebuffer creation failed");
  }
}

void RenderPassHandler::DestroyFrameBuffers(RenderPass& render_pass) const {
  for (auto& render_target : render_pass.render_targets) {
    if (render_target.framebuffer_handle != VK_NULL_HANDLE) {
      vkDestroyFramebuffer(context_->GetDevice(),
                           render_target.framebuffer_handle, nullptr);
      render_target.framebuffer_handle = VK_NULL_HANDLE;
    }
  }
}

void RenderPassHandler::Refresh(RenderPass& render_pass) {
  if (render_pass.render_targets.IsEmpty()) {
    return;
  }

  DestroyFrameBuffers(render_pass);

  const auto& swapchain_images{swapchain_->GetImages()};
  const auto image_count{swapchain_->GetImageCount()};

  COMET_ASSERT(render_pass.render_targets.GetSize() == image_count,
               "RenderPassHandler::Refresh",
               "render target count does not match swapchain image count",
               "render_target_count", render_pass.render_targets.GetSize(),
               "image_count", image_count);

  const auto is_msaa{context_->GetDevice().IsMsaa()};

  for (u32 i{0}; i < image_count; ++i) {
    auto& render_target{render_pass.render_targets[i]};
    const auto swapchain_image_view{swapchain_images[i].image_view_handle};

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

      COMET_ASSERT(false, "RenderPassHandler::Refresh",
                   "attachment type is unsupported", "attachment_type",
                   GetAttachmentTypeLabel(attachment.type),
                   "attachment_type_value", ToUnderlying(attachment.type));
    }
  }

  GenerateFrameBuffers(render_pass);
}

HashValue RenderPassHandler::GenerateHash(const RenderPassDescr& descr) {
  HashValue hash{0};

  hash = HashCombine(hash, comet::GenerateHash(descr.clear_flags));
  hash = HashCombine(hash, comet::GenerateHash(descr.options));
  hash = HashCombine(hash, comet::GenerateHash(descr.extent.width));
  hash = HashCombine(hash, comet::GenerateHash(descr.extent.height));
  hash = HashCombine(hash, comet::GenerateHash(descr.offset.x));
  hash = HashCombine(hash, comet::GenerateHash(descr.offset.y));

  for (const auto& dependency : descr.dependencies) {
    hash = HashCombine(hash, comet::GenerateHash(dependency.srcSubpass));
    hash = HashCombine(hash, comet::GenerateHash(dependency.dstSubpass));
    hash = HashCombine(
        hash, comet::GenerateHash(static_cast<u32>(dependency.srcStageMask)));
    hash = HashCombine(
        hash, comet::GenerateHash(static_cast<u32>(dependency.dstStageMask)));
    hash = HashCombine(
        hash, comet::GenerateHash(static_cast<u32>(dependency.srcAccessMask)));
    hash = HashCombine(
        hash, comet::GenerateHash(static_cast<u32>(dependency.dstAccessMask)));
    hash = HashCombine(hash, comet::GenerateHash(
                                 static_cast<u32>(dependency.dependencyFlags)));
  }

  for (const auto& attachment : descr.attachment_descrs) {
    hash = HashCombine(hash, comet::GenerateHash(attachment.is_final_layout));
    hash = HashCombine(hash,
                       comet::GenerateHash(static_cast<u32>(attachment.type)));
    hash = HashCombine(
        hash, comet::GenerateHash(static_cast<u32>(attachment.samples)));
    hash = HashCombine(
        hash, comet::GenerateHash(static_cast<u32>(attachment.format)));
    hash = HashCombine(
        hash, comet::GenerateHash(static_cast<u32>(attachment.load_op)));
    hash = HashCombine(
        hash, comet::GenerateHash(static_cast<u32>(attachment.store_op)));
  }

  return hash;
}
}  // namespace vk
}  // namespace render
}  // namespace comet