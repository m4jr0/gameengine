// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_RENDER_PASS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_RENDER_PASS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/rendering/rendering_handle.h"

namespace comet {
namespace rendering {
namespace vk {
enum class AttachmentType : u8 { Unknown = 0, Color, Depth, Stencil, Resolve };

struct AttachmentDescr {
  bool is_final_layout{false};
  AttachmentType type{AttachmentType::Unknown};
  VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM};
  VkFormat format{VK_FORMAT_UNDEFINED};
  VkAttachmentLoadOp load_op{VK_ATTACHMENT_LOAD_OP_DONT_CARE};
  VkAttachmentStoreOp store_op{VK_ATTACHMENT_STORE_OP_DONT_CARE};
};

struct Attachment {
  AttachmentType type{AttachmentType::Unknown};
  VkAttachmentDescription attachment_descr{};
  VkImageView image_view_handle{VK_NULL_HANDLE};
};

struct VulkanRenderTarget {
  VkFramebuffer framebuffer_handle{VK_NULL_HANDLE};
  Array<Attachment> attachments{};
};

using RenderPassClearFlags = u8;

enum RenderPassClearFlagBits : RenderPassClearFlags {
  kRenderPassClearFlagBitsNone = 0x0,
  kRenderPassClearFlagBitsColorBuffer = 0x1,
  kRenderPassClearFlagBitsDepthBuffer = 0x2,
  kRenderPassClearFlagBitsStencilBuffer = 0x4
};

using RenderPassCacheKey = u64;
constexpr auto kInvalidRenderPassCacheKey{static_cast<RenderPassCacheKey>(0)};

enum RenderPassOptionFlagBits {
  kRenderPassOptionFlagBitsNone = 0x0,
  kRenderPassOptionFlagBitsMultisampled = 0x1,
  kRenderPassOptionFlagBitsSwapchainTarget = 0x2,
};

using RenderPassOptionFlags = u8;

struct RenderPassDescr {
  RenderPassClearFlags clear_flags{kRenderPassClearFlagBitsNone};
  RenderPassOptionFlags options{kRenderPassOptionFlagBitsNone};
  RenderPassHandle handle{};
  VkExtent2D extent{};
  VkOffset2D offset{};
  Array<VkSubpassDependency> dependencies{};
  Array<AttachmentDescr> attachment_descrs{};
};

struct RenderPass {
  bool is_shared{false};
  RenderPassClearFlags clear_flags{kRenderPassClearFlagBitsNone};
  VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};
  VkExtent2D extent{};
  VkOffset2D offset{};
  RenderPassHandle handle{};
  VkRenderPass vk_handle{VK_NULL_HANDLE};
  u32 ref_count{0};
  RenderPassCacheKey cache_key{kInvalidRenderPassCacheKey};
  Array<VulkanRenderTarget> render_targets{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_RENDER_PASS_H_
