// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_shader_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/rendering/driver/vulkan/utils/vulkan_texture_map_utils.h"

namespace comet {
namespace rendering {
namespace vk {
void AddBufferBinding(frame::FrameArray<ShaderBufferBindingUpdate>& updates,
                      ShaderBindingIndex binding_index, VkBuffer buffer_handle,
                      VkDeviceSize buffer_size, VkDeviceSize buffer_offset) {
  auto& update{updates.EmplaceBack()};
  update.binding_index = binding_index;
  update.buffer_handle = buffer_handle;
  update.buffer_size = buffer_size;
  update.buffer_offset = buffer_offset;
}

void AddImageBinding(frame::FrameArray<ShaderImageBindingUpdate>& updates,
                     ShaderBindingIndex binding_index,
                     const ShaderImageDescriptor* descriptors,
                     u32 descriptor_count) {
  auto& update{updates.EmplaceBack()};
  update.binding_index = binding_index;
  update.descriptors = descriptors;
  update.descriptor_count = descriptor_count;
}

void AddFieldUpdate(frame::FrameArray<ShaderBufferFieldUpdate>& updates,
                    ShaderBindingIndex binding_index,
                    ShaderFieldIndex field_index, const void* data,
                    usize size) {
  auto& update{updates.EmplaceBack()};
  update.binding_index = binding_index;
  update.field_index = field_index;
  update.data = data;
  update.size = size;
}

ShaderImageDescriptor GenerateImageDescriptor(ShaderBindingType binding_type,
                                              const TextureMap& texture_map) {
  ShaderImageDescriptor descriptor{};

  switch (binding_type) {
    case ShaderBindingType::CombinedImageSampler:
      COMET_ASSERT(texture_map.texture != nullptr,
                   "CombinedImageSampler requires a texture!");
      COMET_ASSERT(texture_map.sampler != nullptr,
                   "CombinedImageSampler requires a sampler!");
      descriptor.texture = texture_map.texture;
      descriptor.sampler = texture_map.sampler;
      descriptor.image_layout = GetDescriptorImageLayout(&texture_map);
      return descriptor;

    case ShaderBindingType::SampledImage:
      COMET_ASSERT(texture_map.texture != nullptr,
                   "SampledImage requires a texture!");
      descriptor.texture = texture_map.texture;
      descriptor.sampler = nullptr;
      descriptor.image_layout = GetDescriptorImageLayout(&texture_map);
      return descriptor;

    case ShaderBindingType::Sampler:
      COMET_ASSERT(texture_map.sampler != nullptr,
                   "Sampler requires a sampler!");
      descriptor.texture = nullptr;
      descriptor.sampler = texture_map.sampler;
      descriptor.image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
      return descriptor;

    case ShaderBindingType::StorageImage:
      COMET_ASSERT(texture_map.texture != nullptr,
                   "StorageImage requires a texture!");
      descriptor.texture = texture_map.texture;
      descriptor.sampler = nullptr;
      descriptor.image_layout = VK_IMAGE_LAYOUT_GENERAL;
      return descriptor;

    default:
      COMET_ASSERT(false, "Unsupported image binding type!");
      return descriptor;
  }
}

void GenerateImageDescriptors(ShaderBindingType binding_type,
                              const Array<const TextureMap*>& texture_maps,
                              Array<ShaderImageDescriptor>& descriptors) {
  COMET_ASSERT(IsImageBindingType(binding_type),
               "Binding type is not an image binding type!");

  descriptors.Clear();
  descriptors.Reserve(texture_maps.GetSize());

  for (const auto* texture_map : texture_maps) {
    COMET_ASSERT(texture_map != nullptr, "Texture map is null!");
    descriptors.PushBack(GenerateImageDescriptor(binding_type, *texture_map));
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet