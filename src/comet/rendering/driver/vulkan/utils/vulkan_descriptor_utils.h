// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_DESCRIPTOR_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_DESCRIPTOR_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"

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
    const Array<VkDescriptorSetLayout>& descriptor_set_layout_handles,
    Array<VkDescriptorSet>& descriptor_set_handles,
    VkDescriptorPool& descriptor_pool_handle);
bool AllocateDescriptor(
    VkDevice device_handle,
    const VkDescriptorSetLayout* descriptor_set_layout_handles,
    VkDescriptorSet* descriptor_set_handles,
    VkDescriptorPool& descriptor_pool_handle, u32 count);
void FreeDescriptor(VkDevice device_handle,
                    VkDescriptorSet descriptor_set_handle,
                    VkDescriptorPool& descriptor_pool_handle);
void FreeDescriptor(VkDevice device_handle,
                    Array<VkDescriptorSet>& descriptor_set_handles,
                    VkDescriptorPool& descriptor_pool_handle);
void FreeDescriptor(VkDevice device_handle,
                    VkDescriptorSet* descriptor_set_handles,
                    VkDescriptorPool& descriptor_pool_handle, u32 count);

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
void SetDescriptorSetLabels(const VkDescriptorSet* set_handles, u32 count,
                            const schar* prefix);
#define COMET_VK_SET_DESCRIPTOR_SET_LABELS(set_handles, count, prefix) \
  SetDescriptorSetLabels(set_handles, count, prefix);
#else
#define COMET_VK_SET_DESCRIPTOR_SET_LABELS(set_handles, count, prefix)
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_DESCRIPTOR_UTILS_H_
