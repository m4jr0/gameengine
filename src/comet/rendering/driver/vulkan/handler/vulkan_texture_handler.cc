// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_texture_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
#include "comet/core/c_string.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/type/string_id.h"
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type_trait.h"
#include "comet/math/math_scalar.h"
#include "comet/rendering/driver/vulkan/label/vulkan_texture_label.h"
#include "comet/rendering/driver/vulkan/type/vulkan_image.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_image_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_texture_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_alloc.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_device.h"
#include "comet/rendering/label/texture_label.h"
#include "comet/rendering/utils/texture_utils.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace rendering {
namespace vk {
TextureHandler::TextureHandler(const TextureHandlerDescr& descr)
    : Handler{descr}, textures_{&cache_allocator_, 1024} {}

TextureHandle TextureHandler::GetOrGenerate(
    resource::TextureResourceId texture_resource_id) {
  return GetOrGenerate(texture_resource_id, TextureType::Unknown);
}

TextureHandle TextureHandler::GetOrGenerate(
    resource::TextureResourceId texture_resource_id, TextureType type) {
  COMET_ASSERT(texture_resource_id.IsValid(), "TextureHandler::GetOrGenerate",
               "texture resource id is invalid");

  TextureKey key{
      .kind = TextureKeyKind::Resource,
      .texture_resource_id = texture_resource_id,
      .runtime_id = kInvalidRuntimeTextureId,
      .type = type,
  };

  if (const auto handle{textures_.TryAcquire(key)}; handle) {
    return handle;
  }

  TextureHandle generated_handle{TextureHandle::Invalid()};
  auto* texture_resource_handler{
      resource::ResourceManager::Get().GetTextures()};

  const auto is_loaded{texture_resource_handler->WithTemporaryLoad(
      texture_resource_id,
      [this, &generated_handle, type,
       texture_resource_id](const resource::TextureResource* texture_resource) {
        auto* texture{GenerateTexture(texture_resource, type)};
        COMET_ASSERT(texture != nullptr, "TextureHandler::GetOrGenerate",
                     "generated texture is null", "texture_resource_id",
                     texture_resource_id);

        TextureKey key{
            .kind = TextureKeyKind::Resource,
            .texture_resource_id = texture->texture_resource_id,
            .runtime_id = texture->runtime_id,
            .type = texture->type,
        };

        generated_handle = textures_.Create(key, texture);
        COMET_ASSERT(generated_handle, "TextureHandler::GetOrGenerate",
                     "texture instance creation failed", "texture_resource_id",
                     texture_resource_id, "texture_type",
                     GetTextureTypeLabel(type), "texture_type_value",
                     ToUnderlying(type));

        texture->handle = generated_handle;
      })};

  return is_loaded ? generated_handle : TextureHandle::Invalid();
}

TextureHandle TextureHandler::Generate(const RuntimeTextureDescr& descr) {
  COMET_ASSERT(descr.width > 0, "TextureHandler::Generate",
               "runtime texture width is zero");
  COMET_ASSERT(descr.height > 0, "TextureHandler::Generate",
               "runtime texture height is zero");
  COMET_ASSERT(descr.format != VK_FORMAT_UNDEFINED, "TextureHandler::Generate",
               "runtime texture format is undefined");
  COMET_ASSERT(descr.usage != 0, "TextureHandler::Generate",
               "runtime texture usage is empty");
  COMET_ASSERT(descr.layer_count > 0, "TextureHandler::Generate",
               "runtime texture layer count is zero");

  auto* texture{allocator_.AllocateOneAndPopulate<Texture>()};
  texture->handle = TextureHandle::Invalid();
  texture->type = descr.type;
  texture->width = descr.width;
  texture->height = descr.height;
  texture->depth = descr.depth;
  texture->mip_levels = descr.mip_levels;
  texture->channel_count = descr.channel_count;
  texture->format = descr.format;
  texture->image.allocator_handle = context_->GetAllocatorHandle();

  COMET_ASSERT(next_runtime_texture_id_ != kInvalidRuntimeTextureId,
               "TextureHandler::Generate", "runtime texture id overflow");

  texture->runtime_id = next_runtime_texture_id_++;
  texture->is_runtime = true;
  texture->texture_resource_id = resource::TextureResourceId::Invalid();

  auto& device{context_->GetDevice()};

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
  const schar* debug_label{descr.debug_label};
#else
  const schar* debug_label{nullptr};
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

  GenerateImage(texture->image, device, descr.width, descr.height,
                descr.mip_levels, descr.layer_count, descr.sample_count,
                descr.format, VK_IMAGE_TILING_OPTIMAL, descr.usage, 0,
                debug_label);

  texture->image.image_view_handle = GenerateImageView(
      device, texture->image.handle, descr.format, descr.aspect_flags,
      descr.mip_levels, 0, descr.layer_count, descr.view_type);

  TransitionImageLayout(*context_, texture->image.handle, descr.format,
                        VK_IMAGE_LAYOUT_UNDEFINED, descr.final_layout,
                        descr.mip_levels, descr.layer_count);

  TextureKey key{
      .kind = TextureKeyKind::Runtime,
      .texture_resource_id = texture->texture_resource_id,
      .runtime_id = texture->runtime_id,
      .type = texture->type,
  };

  const auto handle{textures_.Create(key, texture)};
  COMET_ASSERT(handle, "TextureHandler::Generate",
               "texture instance creation failed", "runtime_texture_id",
               texture->runtime_id, "texture_type",
               GetTextureTypeLabel(texture->type), "texture_type_value",
               ToUnderlying(texture->type));

  texture->handle = handle;
  return handle;
}

void TextureHandler::Destroy(TextureHandle handle) {
  auto* texture{textures_.Get(handle)};

  if (!textures_.Release(handle)) {
    return;
  }

  DestroyTexture(texture);
  textures_.Remove(handle);
}

const Texture* TextureHandler::Get(TextureHandle handle) const {
  const auto* texture{textures_.TryGet(handle)};
  COMET_ASSERT(texture != nullptr, "TextureHandler::Get", "texture not found",
               "texture_handle", handle);
  return texture;
}

void TextureHandler::OnInitialize() {
  allocator_.Initialize();
  textures_.Initialize();
}

void TextureHandler::OnShutdown() {
  memory::PlatformAllocator tmp_allocator{memory::kEngineMemoryTagRendering};
  auto handles_to_destroy{Array<TextureHandle>::WithCapacity(
      &tmp_allocator, textures_.GetLiveCount())};

  textures_.ForEachLive(
      [&handles_to_destroy](TextureHandle handle, const Texture*) {
        handles_to_destroy.PushLast(handle);
      });

  for (const auto handle : handles_to_destroy) {
    const auto ref_count{textures_.GetRefCount(handle)};

    if (ref_count > 0) {
      const auto* texture{textures_.Get(handle)};

      [[maybe_unused]] TextureKey key{
          .kind = texture->is_runtime ? TextureKeyKind::Runtime
                                      : TextureKeyKind::Resource,
          .texture_resource_id = texture->texture_resource_id,
          .runtime_id = texture->runtime_id,
          .type = texture->type,
      };

      COMET_LOG_WARNING(LoggerType::Rendering, "TextureHandler::OnShutdown",
                        "forcing texture destruction", "texture_handle", handle,
                        "ref_count", ref_count, "key_kind",
                        GetTextureKeyKindLabel(key.kind), "key_kind_value",
                        ToUnderlying(key.kind), "texture_resource_id",
                        key.texture_resource_id, "runtime_id", key.runtime_id,
                        "texture_type", GetTextureTypeLabel(key.type),
                        "texture_type_value", ToUnderlying(key.type));
    }

    auto* texture{textures_.Drain(handle)};

    if (texture == nullptr) {
      continue;
    }

    COMET_ASSERT(texture->handle == handle, "TextureHandler::OnShutdown",
                 "texture handle mismatch", "expected_handle", handle,
                 "actual_handle", texture->handle, "texture_resource_id",
                 texture->texture_resource_id, "runtime_id",
                 texture->runtime_id);

    DestroyTexture(texture);
  }

  textures_.Destroy();
  allocator_.Destroy();
}

Texture* TextureHandler::Get(TextureHandle handle) {
  auto* texture{textures_.TryGet(handle)};
  COMET_ASSERT(texture != nullptr, "TextureHandler::Get", "texture not found",
               "texture_handle", handle);
  return texture;
}

Texture* TextureHandler::GenerateTexture(
    const resource::TextureResource* resource, TextureType type) {
  COMET_ASSERT(resource != nullptr, "TextureHandler::GenerateTexture",
               "texture resource is null");

  auto* texture{allocator_.AllocateOneAndPopulate<Texture>()};
  texture->handle = TextureHandle::Invalid();
  const auto& resource_descr{resource->descr};

  texture->type = type;
  texture->width = resource_descr.resolution[0];
  texture->height = resource_descr.resolution[1];
  texture->depth = resource_descr.resolution[2];
  texture->mip_levels = GetMipLevels(texture->width, texture->height);
  texture->channel_count = GetResolvedChannelCount(
      resource_descr.format, static_cast<u8>(resource_descr.channel_count));
  texture->format = GetVkFormat(resource_descr.format, type);
  texture->image.allocator_handle = context_->GetAllocatorHandle();

  texture->runtime_id = kInvalidRuntimeTextureId;
  texture->is_runtime = false;
  texture->texture_resource_id = resource->GetId();

  const auto image_size{static_cast<VkDeviceSize>(
      texture->width * texture->height * texture->channel_count)};

  auto& device{context_->GetDevice()};

  auto staging_buffer{
      GenerateBuffer(context_->GetAllocatorHandle(), image_size,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                     VK_SHARING_MODE_EXCLUSIVE, "texture_staging_buffer")};

  {
    ScopedMappedBuffer mapped{staging_buffer};
    CopyToBuffer(staging_buffer, resource->data.GetData(),
                 static_cast<usize>(image_size));
  }

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
  constexpr auto kDebugLabelLen{kMaxPathLength};
  schar debug_label[kDebugLabelLen + 1]{'\0'};

  constexpr schar kPrefix[]{"image_"};
  constexpr auto kPrefixLen{GetLength(kPrefix)};

  Copy(debug_label, kPrefix, kPrefixLen);

  const auto* resource_label{
      COMET_STRING_ID_LABEL(resource->GetId().GetValue())};

  const auto resource_label_len{
      math::Min(GetLength(resource_label), kDebugLabelLen - kPrefixLen)};

  Copy(debug_label + kPrefixLen, resource_label, resource_label_len);
  debug_label[kPrefixLen + resource_label_len] = '\0';
#else
  const schar* debug_label{nullptr};
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

  GenerateImage(texture->image, device, texture->width, texture->height,
                texture->mip_levels, 1, VK_SAMPLE_COUNT_1_BIT, texture->format,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, debug_label);

  TransitionImageLayout(*context_, texture->image.handle, texture->format,
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        texture->mip_levels, 1);

  const auto command_pool_handle{context_->GetFrameData().command_pool_handle};
  auto command_buffer_handle{
      GenerateOneTimeCommand(device, command_pool_handle)};

  CopyBufferToImage(command_buffer_handle, staging_buffer, texture->image,
                    texture->width, texture->height);

  SubmitOneTimeCommand(command_buffer_handle, command_pool_handle, device,
                       device.GetGraphicsQueueHandle());

  DestroyBuffer(staging_buffer);

  GenerateMipmaps(texture);

  texture->image.image_view_handle =
      GenerateImageView(device, texture->image.handle, texture->format,
                        VK_IMAGE_ASPECT_COLOR_BIT, texture->mip_levels);

  return texture;
}

void TextureHandler::DestroyTexture(Texture* texture) {
  COMET_ASSERT(texture != nullptr, "TextureHandler::DestroyTexture",
               "texture is null");

  if (texture->image.image_view_handle != VK_NULL_HANDLE) {
    vkDestroyImageView(context_->GetDevice(), texture->image.image_view_handle,
                       MemoryCallbacks::Get().GetAllocCallbacksHandle());
    texture->image.image_view_handle = VK_NULL_HANDLE;
  }

  DestroyImage(texture->image);
  texture->handle.Invalidate();
  allocator_.Deallocate(texture);
}

void TextureHandler::GenerateMipmaps(Texture* texture) const {
  COMET_ASSERT(texture != nullptr, "TextureHandler::GenerateMipmaps",
               "texture is null");

  auto& device{context_->GetDevice()};

  VkFormatProperties format_properties{};
  vkGetPhysicalDeviceFormatProperties(device.GetPhysicalDeviceHandle(),
                                      texture->format, &format_properties);

  COMET_ASSERT(
      static_cast<bool>(format_properties.optimalTilingFeatures &
                        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT),
      "TextureHandler::GenerateMipmaps",
      "texture format does not support linear blitting", "format_value",
      ToUnderlying(texture->format), "texture_resource_id",
      texture->texture_resource_id, "runtime_id", texture->runtime_id);

  const auto command_pool_handle{context_->GetFrameData().command_pool_handle};
  auto command_buffer_handle{
      GenerateOneTimeCommand(device, command_pool_handle)};

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
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);

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
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &barrier);

    if (mip_width > 1) {
      mip_width /= 2;
    }

    if (mip_height > 1) {
      mip_height /= 2;
    }
  }

  barrier.subresourceRange.baseMipLevel = texture->mip_levels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(command_buffer_handle, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);

  SubmitOneTimeCommand(command_buffer_handle, command_pool_handle, device,
                       device.GetGraphicsQueueHandle());
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet