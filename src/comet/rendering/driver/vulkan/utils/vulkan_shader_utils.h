// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_SHADER_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_SHADER_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/type/vulkan_shader_type.h"
#include "comet/rendering/driver/vulkan/type/vulkan_texture_map_type.h"

namespace comet {
namespace rendering {
namespace vk {
VkShaderStageFlags ResolveStageFlags(ShaderStageFlags flags);

bool IsGraphicsStage(VkShaderStageFlags stage_flags);
bool IsComputeStage(VkShaderStageFlags stage_flags);

void AddBufferBinding(frame::FrameArray<ShaderBufferBindingUpdate>& updates,
                      ShaderBindingIndex binding_index, VkBuffer buffer_handle,
                      VkDeviceSize buffer_size, VkDeviceSize buffer_offset = 0);

void AddImageBinding(frame::FrameArray<ShaderImageBindingUpdate>& updates,
                     ShaderBindingIndex binding_index,
                     const ShaderImageDescriptor* descriptors,
                     u32 descriptor_count);

void AddFieldUpdate(frame::FrameArray<ShaderBufferFieldUpdate>& updates,
                    ShaderBindingIndex binding_index,
                    ShaderFieldIndex field_index, const void* data,
                    usize size = 0);

ShaderImageDescriptor GenerateImageDescriptor(ShaderBindingType binding_type,
                                              TextureHandle texture_handle,
                                              SamplerHandle sampler_handle,
                                              VkImageLayout image_layout);
void GenerateImageDescriptors(ShaderBindingType binding_type,
                              const Array<const TextureMap*>& texture_maps,
                              const Array<VkImageLayout>& image_layouts,
                              Array<ShaderImageDescriptor>& descriptors);

Alignment GetBindingFieldAlignment(ShaderMemoryLayout layout,
                                   ShaderVariableType type);
ShaderOffset AlignOffset(ShaderOffset offset, Alignment alignment);

ShaderFieldLayoutInfo GetFieldLayoutInfo(ShaderMemoryLayout layout,
                                         ShaderVariableType type,
                                         u32 array_count);

VkCompareOp GetVkCompareOp(CompareOp op);
VkDescriptorType GetVkDescriptorType(ShaderBindingType type);
VkShaderStageFlagBits GetVkStage(ShaderStage stage);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_SHADER_UTILS_H_
