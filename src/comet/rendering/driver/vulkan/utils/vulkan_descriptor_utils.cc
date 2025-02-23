// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_descriptor_utils.h"

#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
VkDescriptorPool GenerateDescriptorPool(VkDevice device_handle,
                                        u32 max_descriptor_set_count,
                                        const VkDescriptorPoolSize* pool_sizes,
                                        u32 pool_size_count,
                                        VkDescriptorPoolCreateFlags flags) {
  VkDescriptorPoolCreateInfo pool_info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      VK_NULL_HANDLE,
      flags,
      max_descriptor_set_count,
      pool_size_count,
      pool_sizes};
  VkDescriptorPool descriptor_pool_handle;

  COMET_CHECK_VK(
      vkCreateDescriptorPool(device_handle, &pool_info, VK_NULL_HANDLE,
                             &descriptor_pool_handle),
      "Could not create descriptor pool!");

  return descriptor_pool_handle;
}

void DestroyDescriptorPool(VkDevice device_handle,
                           VkDescriptorPool& descriptor_pool_handle) {
  if (descriptor_pool_handle == VK_NULL_HANDLE) {
    return;
  }

  vkDestroyDescriptorPool(device_handle, descriptor_pool_handle,
                          VK_NULL_HANDLE);
  descriptor_pool_handle = VK_NULL_HANDLE;
}

bool AllocateDescriptor(VkDevice device_handle,
                        VkDescriptorSetLayout descriptor_set_layout_handle,
                        VkDescriptorSet& descriptor_set_handle,
                        VkDescriptorPool& descriptor_pool_handle) {
  return AllocateDescriptor(device_handle, &descriptor_set_layout_handle,
                            &descriptor_set_handle, descriptor_pool_handle, 1);
}

bool AllocateDescriptor(
    VkDevice device_handle,
    const Array<VkDescriptorSetLayout>& descriptor_set_layout_handles,
    Array<VkDescriptorSet>& descriptor_set_handles,
    VkDescriptorPool& descriptor_pool_handle) {
  return AllocateDescriptor(
      device_handle, descriptor_set_layout_handles.GetData(),
      descriptor_set_handles.GetData(), descriptor_pool_handle,
      static_cast<u32>(descriptor_set_layout_handles.GetSize()));
}

bool AllocateDescriptor(
    VkDevice device_handle,
    const VkDescriptorSetLayout* descriptor_set_layout_handles,
    VkDescriptorSet* descriptor_set_handles,
    VkDescriptorPool& descriptor_pool_handle, u32 count) {
  VkDescriptorSetAllocateInfo allocate_info{};
  allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocate_info.pSetLayouts = descriptor_set_layout_handles;
  allocate_info.descriptorSetCount = count;
  allocate_info.descriptorPool = descriptor_pool_handle;
  allocate_info.pNext = VK_NULL_HANDLE;

  const auto result{vkAllocateDescriptorSets(device_handle, &allocate_info,
                                             descriptor_set_handles)};

  switch (result) {
    case VK_SUCCESS:
      return true;

    case VK_ERROR_FRAGMENTED_POOL:
      COMET_ASSERT(false,
                   "Could not allocate descriptor sets! Pool is fragmented!");
      return false;

    case VK_ERROR_OUT_OF_POOL_MEMORY:
      COMET_ASSERT(false,
                   "Could not allocate descriptor sets! Out of pool memory!");
      return false;

    default:
      COMET_ASSERT(false, "Could not allocate descriptor sets!");
      return false;
  }
}

void FreeDescriptor(VkDevice device_handle,
                    VkDescriptorSet descriptor_set_handle,
                    VkDescriptorPool& descriptor_pool_handle) {
  return FreeDescriptor(device_handle, &descriptor_set_handle,
                        descriptor_pool_handle, 1);
}

void FreeDescriptor(VkDevice device_handle,
                    Array<VkDescriptorSet>& descriptor_set_handles,
                    VkDescriptorPool& descriptor_pool_handle) {
  return FreeDescriptor(device_handle, descriptor_set_handles.GetData(),
                        descriptor_pool_handle,
                        static_cast<u32>(descriptor_set_handles.GetSize()));
}

void FreeDescriptor(VkDevice device_handle,
                    VkDescriptorSet* descriptor_set_handles,
                    VkDescriptorPool& descriptor_pool_handle, u32 count) {
  COMET_CHECK_VK(vkFreeDescriptorSets(device_handle, descriptor_pool_handle,
                                      count, descriptor_set_handles),
                 "Unable to free descriptor sets!");

  for (u32 i{0}; i < count; ++i) {
    descriptor_set_handles[0] = VK_NULL_HANDLE;
  }

  descriptor_pool_handle = VK_NULL_HANDLE;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet