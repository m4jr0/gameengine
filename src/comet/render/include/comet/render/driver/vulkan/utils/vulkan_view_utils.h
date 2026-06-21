// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_VIEW_UTILS_H_
#define COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_VIEW_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/container/array.h"
#include "comet/render/driver/vulkan/type/vulkan_render_pass.h"
#include "comet/render/driver/vulkan/view/vulkan_view.h"

namespace comet {
namespace rendering {
namespace vk {
void GenerateAttachmentDescrs(const ViewPassDescr& pass_descr,
                              VkSampleCountFlagBits samples,
                              Array<AttachmentDescr>& attachment_descrs);

AttachmentDescr GenerateDepthAttachmentDescr(
    ViewLoadOp load_op, ViewStoreOp store_op,
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

u8 GenerateClearFlags(const ViewPassDescr& pass_descr);

VkAttachmentLoadOp ToVkAttachmentLoadOp(ViewLoadOp op);
VkAttachmentStoreOp ToVkAttachmentStoreOp(ViewStoreOp op);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_VIEW_UTILS_H_
