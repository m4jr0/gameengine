// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_TEXTURE_TYPE_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_TEXTURE_TYPE_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/type/vulkan_image_type.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/rendering/type/rendering_texture_type.h"
#include "comet/resource/texture/texture_resource.h"

namespace comet {
namespace rendering {
namespace vk {
enum class TextureKeyKind : u8 { Resource = 0, Runtime };

using RuntimeTextureId = u64;
constexpr auto kInvalidRuntimeTextureId{static_cast<RuntimeTextureId>(-1)};

struct TextureKey {
  TextureKeyKind kind{TextureKeyKind::Resource};
  resource::TextureResourceId texture_resource_id{};
  RuntimeTextureId runtime_id{kInvalidRuntimeTextureId};
  TextureType type{TextureType::Unknown};

  friend bool operator==(const TextureKey& lhs,
                         const TextureKey& rhs) noexcept = default;
};

struct Texture {
  bool is_runtime{false};
  resource::TextureResourceId texture_resource_id{};
  RuntimeTextureId runtime_id{kInvalidRuntimeTextureId};

  TextureHandle handle{};
  TextureType type{TextureType::Unknown};
  u32 width{0};
  u32 height{0};
  u32 depth{0};
  u32 mip_levels{1};
  u8 channel_count{0};
  VkFormat format{VK_FORMAT_UNDEFINED};
  Image image{};
};

struct RuntimeTextureDescr {
  TextureType type{TextureType::Unknown};
  u32 width{0};
  u32 height{0};
  u32 depth{1};
  u32 mip_levels{1};
  u8 channel_count{1};
  VkFormat format{VK_FORMAT_UNDEFINED};

  VkImageUsageFlags usage{0};
  VkImageAspectFlags aspect_flags{VK_IMAGE_ASPECT_COLOR_BIT};
  VkImageViewType view_type{VK_IMAGE_VIEW_TYPE_2D};

  u32 layer_count{1};
  VkSampleCountFlagBits sample_count{VK_SAMPLE_COUNT_1_BIT};

  VkImageLayout final_layout{VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
  const schar* debug_label{nullptr};
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_TEXTURE_TYPE_H_