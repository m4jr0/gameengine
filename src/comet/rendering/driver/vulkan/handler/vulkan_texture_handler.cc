// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_texture_handler.h"

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
#include "comet/core/c_string.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/type/string_id.h"
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

#include "comet/core/memory/allocator/allocator.h"
#include "comet/math/math_common.h"
#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/data/vulkan_image.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_image_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_alloc.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_device.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace rendering {
namespace vk {
TextureHandler::TextureHandler(const TextureHandlerDescr& descr)
    : Handler{descr} {}

void TextureHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();
  textures_ = Map<TextureId, Texture*>{&allocator_};
}

void TextureHandler::Shutdown() {
  for (auto& it : textures_) {
    Destroy(it.value, true);
  }

  textures_.Destroy();
  allocator_.Destroy();
  Handler::Shutdown();
}

const Texture* TextureHandler::Generate(
    const resource::TextureResource* resource) {
  auto* texture{
      textures_.Emplace(resource->id, GenerateInstance(resource)).value};

  // TODO(m4jr0): Support compatibility with GPU properly.
  texture->channel_count = 4;
  texture->format = VK_FORMAT_R8G8B8A8_SRGB;

  // Generate texture image.
  const auto image_size{texture->width * texture->height *
                        texture->channel_count};
  Buffer staging_buffer{};
  auto& device{context_->GetDevice()};

  staging_buffer =
      GenerateBuffer(context_->GetAllocatorHandle(), image_size,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                     VK_SHARING_MODE_EXCLUSIVE, "staging_buffer");

  MapBuffer(staging_buffer);
  CopyToBuffer(staging_buffer, resource->data.GetData(), image_size);
  UnmapBuffer(staging_buffer);

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
  constexpr auto kDebugLabelLen{kMaxPathLength};
  schar debug_label[kDebugLabelLen + 1]{'\0'};
  auto image_len{GetLength("image_")};
  Copy(debug_label, "image_", image_len);
  auto* resource_label{COMET_STRING_ID_LABEL(resource->id)};
  Copy(debug_label + image_len, resource_label, GetLength(resource_label));
#else
  const schar* debug_label{nullptr};
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

  GenerateImage(texture->image, device, texture->width, texture->height,
                texture->mip_levels, VK_SAMPLE_COUNT_1_BIT, texture->format,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, debug_label);

  auto old_layout{VK_IMAGE_LAYOUT_UNDEFINED};
  auto new_layout{VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL};
  auto command_pool_handle{context_->GetFrameData().command_pool_handle};

  TransitionImageLayout(*context_, texture->image.handle, texture->format,
                        old_layout, new_layout, texture->mip_levels);
  auto command_buffer_handle{
      GenerateOneTimeCommand(device, command_pool_handle)};
  CopyBufferToImage(command_buffer_handle, staging_buffer, texture->image,
                    texture->width, texture->height);
  SubmitOneTimeCommand(command_buffer_handle, command_pool_handle, device,
                       device.GetGraphicsQueueHandle());
  DestroyBuffer(staging_buffer);
  GenerateMipmaps(texture);

  // Generate texture image view.
  texture->image.image_view_handle =
      GenerateImageView(device, texture->image.handle, texture->format,
                        VK_IMAGE_ASPECT_COLOR_BIT, texture->mip_levels);
  return texture;
}

const Texture* TextureHandler::Get(TextureId texture_id) const {
  const auto* texture{TryGet(texture_id)};
  COMET_ASSERT(texture != nullptr,
               "Requested texture does not exist: ", texture_id, "!");
  return texture;
}

const Texture* TextureHandler::TryGet(TextureId texture_id) const {
  auto texture_ptr{textures_.TryGet(texture_id)};

  if (texture_ptr == nullptr) {
    return nullptr;
  }

  auto* texture{*texture_ptr};
  ++texture->ref_count;
  return texture;
}

const Texture* TextureHandler::GetOrGenerate(
    const resource::TextureResource* resource) {
  const auto* texture{TryGet(resource->id)};

  if (texture != nullptr) {
    return texture;
  }

  return Generate(resource);
}

void TextureHandler::Destroy(TextureId texture_id) {
  return Destroy(Get(texture_id));
}

void TextureHandler::Destroy(Texture* texture) {
  return Destroy(texture, false);
}

Texture* TextureHandler::Get(TextureId texture_id) {
  auto* texture{TryGet(texture_id)};
  COMET_ASSERT(texture != nullptr,
               "Requested texture does not exist: ", texture_id, "!");
  return texture;
}

Texture* TextureHandler::TryGet(TextureId texture_id) {
  auto** texture{textures_.TryGet(texture_id)};

  if (texture == nullptr) {
    return nullptr;
  }

  return *texture;
}

void TextureHandler::Destroy(Texture* texture, bool is_destroying_handler) {
  if (!is_destroying_handler) {
    COMET_ASSERT(texture->ref_count > 0, "Texture has a reference count of 0!");

    if (--texture->ref_count > 0) {
      return;
    }
  }

  if (texture->image.image_view_handle != VK_NULL_HANDLE) {
    vkDestroyImageView(context_->GetDevice(), texture->image.image_view_handle,
                       MemoryCallbacks::Get().GetAllocCallbacksHandle());
  }

  DestroyImage(texture->image);

  if (!is_destroying_handler) {
    textures_.Remove(texture->id);
  }

  allocator_.Deallocate(texture);
}

u32 TextureHandler::GetMipLevels(const resource::TextureResource* resource) {
  return static_cast<u32>(math::Log2(math::Max(
             resource->descr.resolution[0], resource->descr.resolution[1]))) +
         1;
}

VkFormat TextureHandler::GetVkFormat(
    const resource::TextureResource* resource) {
  switch (resource->descr.format) {
    case (rendering::TextureFormat::Rgba8):
      return VK_FORMAT_R8G8B8A8_SRGB;
      break;
    case (rendering::TextureFormat::Rgb8):
      return VK_FORMAT_R8G8B8_SRGB;
    default:
      return VK_FORMAT_UNDEFINED;
  }
}

void TextureHandler::GenerateMipmaps(const Texture* texture) const {
  auto& device{context_->GetDevice()};

  // Check if image format supports linear blitting.
  VkFormatProperties format_properties;
  vkGetPhysicalDeviceFormatProperties(device.GetPhysicalDeviceHandle(),
                                      texture->format, &format_properties);

  COMET_ASSERT(
      static_cast<bool>(format_properties.optimalTilingFeatures &
                        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT),
      "Texture image format does not support linear blitting");

  // vkCmdBlitImage is only in a queue with graphics capacility.
  auto command_pool_handle{context_->GetFrameData().command_pool_handle};
  auto command_buffer_handle{
      GenerateOneTimeCommand(device, command_pool_handle)};

  // Will be reused several times.
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = texture->image.handle;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  auto mip_width{texture->width};
  auto mip_height{texture->height};

  for (u32 mip_level{1}; mip_level < texture->mip_levels; ++mip_level) {
    barrier.subresourceRange.baseMipLevel = mip_level - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer_handle, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, VK_NULL_HANDLE,
                         0, VK_NULL_HANDLE, 1, &barrier);

    VkImageBlit blit{};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {static_cast<s32>(mip_width),
                          static_cast<s32>(mip_height), 1};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = mip_level - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {mip_width > 1 ? static_cast<s32>(mip_width / 2) : 1,
                          mip_height > 1 ? static_cast<s32>(mip_height / 2) : 1,
                          1};
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = mip_level;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;

    vkCmdBlitImage(command_buffer_handle, texture->image.handle,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->image.handle,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                   VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer_handle, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                         VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);

    if (mip_width > 1) mip_width /= 2;
    if (mip_height > 1) mip_height /= 2;
  }

  barrier.subresourceRange.baseMipLevel = texture->mip_levels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(command_buffer_handle, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                       VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);

  SubmitOneTimeCommand(command_buffer_handle, command_pool_handle, device,
                       device.GetGraphicsQueueHandle());
}

Texture* TextureHandler::GenerateInstance(
    const resource::TextureResource* resource) {
  auto* texture{allocator_.AllocateOneAndPopulate<Texture>()};
  texture->id = resource->id;
  texture->width = resource->descr.resolution[0];
  texture->height = resource->descr.resolution[1];
  texture->depth = resource->descr.resolution[2];
  texture->mip_levels = GetMipLevels(resource);
  texture->format = GetVkFormat(resource);
  texture->channel_count = resource->descr.channel_count;
  texture->image.allocator_handle = context_->GetAllocatorHandle();
  return texture;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
