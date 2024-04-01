// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_DESCRIPTOR_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_DESCRIPTOR_UTILS_H_

#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader_data.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"

namespace comet {
namespace rendering {
namespace vk {
VkDescriptorPool GenerateDescriptorPool(VkDevice device_handle,
                                        u32 max_descriptor_set_count,
                                        const VkDescriptorPoolSize* pool_sizes,
                                        u32 pool_size_count,
                                        VkDescriptorPoolCreateFlags flags);
void DestroyDescriptorPool(VkDevice device_handle,
                           VkDescriptorPool& descriptor_pool_handle);
bool AllocateDescriptor(VkDevice device_handle,
                        VkDescriptorSetLayout descriptor_set_layout_handle,
                        VkDescriptorSet& descriptor_set_handle,
                        VkDescriptorPool& descriptor_pool_handle);
bool AllocateDescriptor(
    VkDevice device_handle,
    const std::vector<VkDescriptorSetLayout>& descriptor_set_layout_handles,
    std::vector<VkDescriptorSet>& descriptor_set_handles,
    VkDescriptorPool& descriptor_pool_handle);
bool AllocateDescriptor(
    VkDevice device_handle,
    const VkDescriptorSetLayout* descriptor_set_layout_handles,
    VkDescriptorSet* descriptor_set_handles,
    VkDescriptorPool& descriptor_pool_handle, u32 count);
bool AllocateShaderUniformData(
    VkDevice device_handle,
    const std::vector<VkDescriptorSetLayout>& descriptor_set_layout_handles,
    ShaderUniformData& shader_uniform_data);
bool AllocateShaderUniformData(
    VkDevice device_handle,
    const VkDescriptorSetLayout* descriptor_set_layout_handles,
    ShaderUniformData& shader_uniform_data);
void FreeDescriptor(VkDevice device_handle,
                    VkDescriptorSet descriptor_set_handle,
                    VkDescriptorPool& descriptor_pool_handle);
void FreeDescriptor(VkDevice device_handle,
                    std::vector<VkDescriptorSet>& descriptor_set_handles,
                    VkDescriptorPool& descriptor_pool_handle);
void FreeDescriptor(VkDevice device_handle,
                    VkDescriptorSet* descriptor_set_handles,
                    VkDescriptorPool& descriptor_pool_handle, u32 count);
void FreeShaderUniformData(VkDevice device_handle,
                           ShaderUniformData& shader_uniform_data);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_DESCRIPTOR_UTILS_H_
