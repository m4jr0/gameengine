// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_view_utils.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
namespace vk {
void GenerateAttachmentDescrs(const ViewPassDescr& pass_descr,
                              VkSampleCountFlagBits samples,
                              Array<AttachmentDescr>& attachment_descrs) {
  const auto has_color{(pass_descr.flags & kViewPassFlagBitsHasColor) != 0};
  const auto has_depth{(pass_descr.flags & kViewPassFlagBitsHasDepth) != 0};
  const auto is_swapchain_target{
      (pass_descr.flags & kViewPassFlagBitsSwapchainTarget) != 0};
  const auto is_msaa{samples != VK_SAMPLE_COUNT_1_BIT};

  if (has_color) {
    AttachmentDescr color_attachment_descr{};
    color_attachment_descr.type = AttachmentType::Color;
    color_attachment_descr.load_op =
        ToVkAttachmentLoadOp(pass_descr.color_load_op);
    color_attachment_descr.store_op =
        ToVkAttachmentStoreOp(pass_descr.color_store_op);
    color_attachment_descr.is_final_layout =
        !is_msaa && is_swapchain_target &&
        pass_descr.final_color_op == ViewFinalColorOp::Present;
    attachment_descrs.PushBack(color_attachment_descr);
  }

  if (has_depth) {
    attachment_descrs.PushBack(GenerateDepthAttachmentDescr(
        pass_descr.depth_load_op, pass_descr.depth_store_op, samples));
  }

  if (has_color && is_msaa) {
    AttachmentDescr resolve_attachment_descr{};
    resolve_attachment_descr.type = AttachmentType::Resolve;
    resolve_attachment_descr.load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolve_attachment_descr.store_op = VK_ATTACHMENT_STORE_OP_STORE;
    resolve_attachment_descr.is_final_layout =
        is_swapchain_target &&
        pass_descr.final_color_op == ViewFinalColorOp::Present;
    attachment_descrs.PushBack(resolve_attachment_descr);
  }
}

AttachmentDescr GenerateDepthAttachmentDescr(ViewLoadOp load_op,
                                             ViewStoreOp store_op,
                                             VkSampleCountFlagBits samples) {
  AttachmentDescr descr{};
  descr.type = AttachmentType::Depth;
  descr.samples = samples;
  descr.load_op = ToVkAttachmentLoadOp(load_op);
  descr.store_op = ToVkAttachmentStoreOp(store_op);
  descr.is_final_layout = false;
  return descr;
}

u8 GenerateClearFlags(const ViewPassDescr& pass_descr) {
  RenderPassClearFlags clear_flags{kRenderPassClearFlagBitsNone};

  if ((pass_descr.flags & kViewPassFlagBitsHasColor) != 0 &&
      pass_descr.color_load_op == ViewLoadOp::Clear) {
    clear_flags |= kRenderPassClearFlagBitsColorBuffer;
  }

  if ((pass_descr.flags & kViewPassFlagBitsHasDepth) != 0 &&
      pass_descr.depth_load_op == ViewLoadOp::Clear) {
    clear_flags |= kRenderPassClearFlagBitsDepthBuffer;
  }

  return clear_flags;
}

VkAttachmentLoadOp ToVkAttachmentLoadOp(ViewLoadOp op) {
  switch (op) {
    case ViewLoadOp::DontCare:
      return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    case ViewLoadOp::Load:
      return VK_ATTACHMENT_LOAD_OP_LOAD;
    case ViewLoadOp::Clear:
      return VK_ATTACHMENT_LOAD_OP_CLEAR;
    default:
      COMET_ASSERT(false, "Unknown view load op!");
      return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  }
}

VkAttachmentStoreOp ToVkAttachmentStoreOp(ViewStoreOp op) {
  switch (op) {
    case ViewStoreOp::DontCare:
      return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    case ViewStoreOp::Store:
      return VK_ATTACHMENT_STORE_OP_STORE;
    default:
      COMET_ASSERT(false, "Unknown view store op!");
      return VK_ATTACHMENT_STORE_OP_DONT_CARE;
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
