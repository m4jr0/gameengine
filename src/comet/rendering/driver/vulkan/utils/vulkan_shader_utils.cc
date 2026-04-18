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

#include "comet/rendering/rendering_utils.h"

namespace comet {
namespace rendering {
namespace vk {
VkShaderStageFlags ResolveStageFlags(ShaderStageFlags flags) {
  VkShaderStageFlags vk_flags{0};

  if (flags & kShaderStageFlagBitsCompute) {
    vk_flags |= VK_SHADER_STAGE_COMPUTE_BIT;
  }

  if (flags & kShaderStageFlagBitsVertex) {
    vk_flags |= VK_SHADER_STAGE_VERTEX_BIT;
  }

  if (flags & kShaderStageFlagBitsFragment) {
    vk_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
  }

  return vk_flags;
}

bool IsGraphicsStage(VkShaderStageFlags stage_flags) {
  return (stage_flags &
          (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) != 0;
}

bool IsComputeStage(VkShaderStageFlags stage_flags) {
  return (stage_flags & VK_SHADER_STAGE_COMPUTE_BIT) != 0;
}

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
                                              TextureHandle texture_handle,
                                              SamplerHandle sampler_handle,
                                              VkImageLayout image_layout) {
  ShaderImageDescriptor descriptor{};

  switch (binding_type) {
    case ShaderBindingType::CombinedImageSampler:
      COMET_ASSERT(texture_handle, "CombinedImageSampler requires a texture!");
      COMET_ASSERT(sampler_handle, "CombinedImageSampler requires a sampler!");
      descriptor.texture_handle = texture_handle;
      descriptor.sampler_handle = sampler_handle;
      descriptor.image_layout = image_layout;
      return descriptor;

    case ShaderBindingType::SampledImage:
      COMET_ASSERT(texture_handle, "SampledImage requires a texture!");
      descriptor.texture_handle = texture_handle;
      descriptor.sampler_handle = {};
      descriptor.image_layout = image_layout;
      return descriptor;

    case ShaderBindingType::Sampler:
      COMET_ASSERT(sampler_handle, "Sampler requires a sampler!");
      descriptor.texture_handle = {};
      descriptor.sampler_handle = sampler_handle;
      descriptor.image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
      return descriptor;

    case ShaderBindingType::StorageImage:
      COMET_ASSERT(texture_handle, "StorageImage requires a texture!");
      descriptor.texture_handle = texture_handle;
      descriptor.sampler_handle = {};
      descriptor.image_layout = VK_IMAGE_LAYOUT_GENERAL;
      return descriptor;

    default:
      COMET_ASSERT(false, "Unsupported image binding type!");
      return descriptor;
  }
}

void GenerateImageDescriptors(ShaderBindingType binding_type,
                              const Array<const TextureMap*>& texture_maps,
                              const Array<VkImageLayout>& image_layouts,
                              Array<ShaderImageDescriptor>& descriptors) {
  COMET_ASSERT(IsImageBindingType(binding_type),
               "Binding type is not an image binding type!");
  COMET_ASSERT(texture_maps.GetSize() == image_layouts.GetSize(),
               "Texture map count and image layout count must match!");

  descriptors.Clear();
  descriptors.Reserve(texture_maps.GetSize());

  for (usize i{0}; i < texture_maps.GetSize(); ++i) {
    const auto* texture_map{texture_maps[i]};
    COMET_ASSERT(texture_map != nullptr, "Texture map is null!");

    descriptors.PushBack(
        GenerateImageDescriptor(binding_type, texture_map->texture_handle,
                                texture_map->sampler_handle, image_layouts[i]));
  }
}

Alignment GetBindingFieldAlignment(ShaderMemoryLayout layout,
                                   ShaderVariableType type) {
  switch (layout) {
    case ShaderMemoryLayout::Std140:
      return GetStd140Alignment(type);
    case ShaderMemoryLayout::Std430:
      return GetStd430Alignment(type);
    case ShaderMemoryLayout::Packed:
      return GetScalarAlignment(type);
    default:
      COMET_ASSERT(false, "Unknown shader memory layout!");
  }

  return kInvalidAlignment;
}

ShaderOffset AlignOffset(ShaderOffset offset, Alignment alignment) {
  return static_cast<ShaderOffset>(
      memory::AlignSize(offset, static_cast<memory::Alignment>(alignment)));
}

ShaderFieldLayoutInfo GetFieldLayoutInfo(ShaderMemoryLayout layout,
                                         ShaderVariableType type,
                                         u32 array_count) {
  ShaderFieldLayoutInfo info{};

  info.element_size = GetShaderVariableTypeSize(type);
  COMET_ASSERT(info.element_size != kInvalidShaderVariableSize,
               "Invalid shader variable type size!");

  info.alignment = GetBindingFieldAlignment(layout, type);

  info.aligned_size = static_cast<ShaderVariableSize>(memory::AlignSize(
      info.element_size, static_cast<memory::Alignment>(info.alignment)));

  info.stride = info.aligned_size;

  if (array_count > 1) {
    info.total_size =
        static_cast<ShaderVariableSize>(info.stride * array_count);
  } else {
    info.total_size = info.aligned_size;
  }

  return info;
}

VkCompareOp GetVkCompareOp(CompareOp op) {
  switch (op) {
    case CompareOp::Never:
      return VK_COMPARE_OP_NEVER;
    case CompareOp::Less:
      return VK_COMPARE_OP_LESS;
    case CompareOp::Equal:
      return VK_COMPARE_OP_EQUAL;
    case CompareOp::LessOrEqual:
      return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareOp::Greater:
      return VK_COMPARE_OP_GREATER;
    case CompareOp::NotEqual:
      return VK_COMPARE_OP_NOT_EQUAL;
    case CompareOp::GreaterOrEqual:
      return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case CompareOp::Always:
      return VK_COMPARE_OP_ALWAYS;
    default:
      COMET_ASSERT(false, "Unknown compare op provided!");
      return VK_COMPARE_OP_LESS;
  }
}

VkDescriptorType GetVkDescriptorType(ShaderBindingType type) {
  switch (type) {
    case ShaderBindingType::UniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case ShaderBindingType::StorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case ShaderBindingType::CombinedImageSampler:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case ShaderBindingType::SampledImage:
      return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case ShaderBindingType::Sampler:
      return VK_DESCRIPTOR_TYPE_SAMPLER;
    case ShaderBindingType::StorageImage:
      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    default:
      COMET_ASSERT(false, "Unsupported shader binding type!");
  }

  return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

VkShaderStageFlagBits GetVkStage(ShaderStage stage) {
  switch (stage) {
    case ShaderStage::Compute:
      return VK_SHADER_STAGE_COMPUTE_BIT;

    case ShaderStage::Vertex:
      return VK_SHADER_STAGE_VERTEX_BIT;

    case ShaderStage::Fragment:
      return VK_SHADER_STAGE_FRAGMENT_BIT;

    default:
      COMET_ASSERT(false, "Unknown shader stage: ",
                   static_cast<std::underlying_type_t<ShaderStage>>(stage),
                   "!");
      return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet