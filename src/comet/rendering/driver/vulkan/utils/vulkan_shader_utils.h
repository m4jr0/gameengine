// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_SHADER_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_SHADER_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader_data.h"
#include "comet/rendering/driver/vulkan/data/vulkan_texture_map.h"

namespace comet {
namespace rendering {
namespace vk {
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
                                              const TextureMap& texture_map);
void GenerateImageDescriptors(ShaderBindingType binding_type,
                              const Array<const TextureMap*>& texture_maps,
                              Array<ShaderImageDescriptor>& descriptors);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_SHADER_UTILS_H_
