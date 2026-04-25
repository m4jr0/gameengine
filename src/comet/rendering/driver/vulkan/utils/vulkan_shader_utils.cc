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

#include "comet/core/type_trait.h"
#include "comet/rendering/label/shader_label.h"
#include "comet/rendering/utils/shader_utils.h"

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
  auto& update{updates.EmplaceLast()};
  update.binding_index = binding_index;
  update.buffer_handle = buffer_handle;
  update.buffer_size = buffer_size;
  update.buffer_offset = buffer_offset;
}

void AddImageBinding(frame::FrameArray<ShaderImageBindingUpdate>& updates,
                     ShaderBindingIndex binding_index,
                     const ShaderImageDescriptor* descriptors,
                     u32 descriptor_count) {
  auto& update{updates.EmplaceLast()};
  update.binding_index = binding_index;
  update.descriptors = descriptors;
  update.descriptor_count = descriptor_count;
}

void AddFieldUpdate(frame::FrameArray<ShaderBufferFieldUpdate>& updates,
                    ShaderBindingIndex binding_index,
                    ShaderFieldIndex field_index, const void* data,
                    usize size) {
  auto& update{updates.EmplaceLast()};
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
      COMET_ASSERT(texture_handle,
                   "vulkan_shader_utils::GenerateImageDescriptor",
                   "combined image sampler requires texture");
      COMET_ASSERT(sampler_handle,
                   "vulkan_shader_utils::GenerateImageDescriptor",
                   "combined image sampler requires sampler");

      descriptor.texture_handle = texture_handle;
      descriptor.sampler_handle = sampler_handle;
      descriptor.image_layout = image_layout;
      return descriptor;

    case ShaderBindingType::SampledImage:
      COMET_ASSERT(texture_handle,
                   "vulkan_shader_utils::GenerateImageDescriptor",
                   "sampled image requires texture");

      descriptor.texture_handle = texture_handle;
      descriptor.sampler_handle = {};
      descriptor.image_layout = image_layout;
      return descriptor;

    case ShaderBindingType::Sampler:
      COMET_ASSERT(sampler_handle,
                   "vulkan_shader_utils::GenerateImageDescriptor",
                   "sampler binding requires sampler");

      descriptor.texture_handle = {};
      descriptor.sampler_handle = sampler_handle;
      descriptor.image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
      return descriptor;

    case ShaderBindingType::StorageImage:
      COMET_ASSERT(texture_handle,
                   "vulkan_shader_utils::GenerateImageDescriptor",
                   "storage image requires texture");

      descriptor.texture_handle = texture_handle;
      descriptor.sampler_handle = {};
      descriptor.image_layout = VK_IMAGE_LAYOUT_GENERAL;
      return descriptor;

    default:
      COMET_ASSERT(false, "vulkan_shader_utils::GenerateImageDescriptor",
                   "image binding type is unsupported", "binding_type",
                   GetShaderBindingTypeLabel(binding_type),
                   "binding_type_value", ToUnderlying(binding_type));

      return descriptor;
  }
}

void GenerateImageDescriptors(ShaderBindingType binding_type,
                              const Array<const TextureMap*>& texture_maps,
                              const Array<VkImageLayout>& image_layouts,
                              Array<ShaderImageDescriptor>& descriptors) {
  COMET_ASSERT(IsImageBindingType(binding_type),
               "vulkan_shader_utils::GenerateImageDescriptors",
               "binding type is not an image binding", "binding_type",
               GetShaderBindingTypeLabel(binding_type), "binding_type_value",
               ToUnderlying(binding_type));
  COMET_ASSERT(texture_maps.GetSize() == image_layouts.GetSize(),
               "vulkan_shader_utils::GenerateImageDescriptors",
               "texture map count and image layout count mismatch",
               "texture_map_count", texture_maps.GetSize(),
               "image_layout_count", image_layouts.GetSize());

  descriptors.Clear();
  descriptors.Reserve(texture_maps.GetSize());

  for (usize i{0}; i < texture_maps.GetSize(); ++i) {
    const auto* texture_map{texture_maps[i]};
    COMET_ASSERT(texture_map != nullptr,
                 "vulkan_shader_utils::GenerateImageDescriptors",
                 "texture map is null", "index", i);

    descriptors.PushLast(
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
      COMET_ASSERT(false, "vulkan_shader_utils::GetBindingFieldAlignment",
                   "shader memory layout is invalid", "layout",
                   GetShaderMemoryLayoutLabel(layout), "layout_value",
                   ToUnderlying(layout));
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
               "vulkan_shader_utils::GetFieldLayoutInfo",
               "shader variable type size is invalid", "type",
               GetShaderVariableTypeLabel(type), "type_value",
               ToUnderlying(type));

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
      COMET_ASSERT(false, "vulkan_shader_utils::GetVkCompareOp",
                   "compare op is invalid", "compare_op", ToUnderlying(op));
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
      COMET_ASSERT(false, "vulkan_shader_utils::GetVkDescriptorType",
                   "shader binding type is unsupported", "binding_type",
                   ToUnderlying(type));
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
      COMET_ASSERT(false, "vulkan_shader_utils::GetVkStage",
                   "shader stage is invalid", "stage", ToUnderlying(stage));
      return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet