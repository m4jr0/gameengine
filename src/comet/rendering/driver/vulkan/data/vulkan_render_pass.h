// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_RENDER_PASS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_RENDER_PASS_H_

#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/string_id.h"

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

enum RenderPassClearFlag {
  None = 0x0,
  ColorBuffer = 0x1,
  DepthBuffer = 0x2,
  StencilBuffer = 0x4
};

using RenderPassId = stringid::StringId;
constexpr auto kInvalidRenderPassId = static_cast<RenderPassId>(-1);

struct RenderPassDescr {
  u8 clear_flags{RenderPassClearFlag::None};
  VkExtent2D extent{};
  VkOffset2D offset{};
  RenderPassId id{kInvalidRenderPassId};
  VkSubpassDependency dependency{};
  Array<AttachmentDescr> attachment_descrs{};
  StaticArray<VkClearValue, 2> clear_values{};
};

struct RenderPass {
  u8 clear_flags{RenderPassClearFlag::None};
  VkExtent2D extent{};
  VkOffset2D offset{};
  RenderPassId id{kInvalidRenderPassId};
  VkRenderPass handle{VK_NULL_HANDLE};
  Array<VulkanRenderTarget> render_targets{};
  StaticArray<VkClearValue, 2> clear_values{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_RENDER_PASS_H_
