// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_descriptor_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/type_trait.h"
#include "comet/math/math_scalar.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
VkDescriptorPool GenerateDescriptorPool(VkDevice device_handle,
                                        u32 max_descriptor_set_count,
                                        const VkDescriptorPoolSize* pool_sizes,
                                        u32 pool_size_count,
                                        VkDescriptorPoolCreateFlags flags) {
  COMET_ASSERT(device_handle != VK_NULL_HANDLE,
               "vulkan_descriptor_utils::GenerateDescriptorPool",
               "device handle is invalid");
  COMET_ASSERT(max_descriptor_set_count > 0,
               "vulkan_descriptor_utils::GenerateDescriptorPool",
               "max descriptor set count is zero");
  COMET_ASSERT(pool_sizes != nullptr,
               "vulkan_descriptor_utils::GenerateDescriptorPool",
               "descriptor pool sizes are null");
  COMET_ASSERT(pool_size_count > 0,
               "vulkan_descriptor_utils::GenerateDescriptorPool",
               "descriptor pool size count is zero");

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
      "vulkan_descriptor_utils::GenerateDescriptorPool",
      "descriptor pool creation failed");

  return descriptor_pool_handle;
}

void DestroyDescriptorPool(VkDevice device_handle,
                           VkDescriptorPool& descriptor_pool_handle) {
  COMET_ASSERT(device_handle != VK_NULL_HANDLE,
               "vulkan_descriptor_utils::DestroyDescriptorPool",
               "device handle is invalid");

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
  COMET_ASSERT(descriptor_set_layout_handle != VK_NULL_HANDLE,
               "vulkan_descriptor_utils::AllocateDescriptor",
               "descriptor set layout handle is invalid");

  return AllocateDescriptor(device_handle, &descriptor_set_layout_handle,
                            &descriptor_set_handle, descriptor_pool_handle, 1);
}

bool AllocateDescriptor(
    VkDevice device_handle,
    const Array<VkDescriptorSetLayout>& descriptor_set_layout_handles,
    Array<VkDescriptorSet>& descriptor_set_handles,
    VkDescriptorPool& descriptor_pool_handle) {
  COMET_ASSERT(!descriptor_set_layout_handles.IsEmpty(),
               "vulkan_descriptor_utils::AllocateDescriptor",
               "descriptor set layout handles are empty");
  COMET_ASSERT(descriptor_set_layout_handles.GetSize() ==
                   descriptor_set_handles.GetSize(),
               "vulkan_descriptor_utils::AllocateDescriptor",
               "descriptor set layout count and descriptor set count mismatch",
               "layout_count", descriptor_set_layout_handles.GetSize(),
               "descriptor_set_count", descriptor_set_handles.GetSize());

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
  COMET_ASSERT(device_handle != VK_NULL_HANDLE,
               "vulkan_descriptor_utils::AllocateDescriptor",
               "device handle is invalid");
  COMET_ASSERT(descriptor_set_layout_handles != nullptr,
               "vulkan_descriptor_utils::AllocateDescriptor",
               "descriptor set layout handles are null");
  COMET_ASSERT(descriptor_set_handles != nullptr,
               "vulkan_descriptor_utils::AllocateDescriptor",
               "descriptor set handles are null");
  COMET_ASSERT(descriptor_pool_handle != VK_NULL_HANDLE,
               "vulkan_descriptor_utils::AllocateDescriptor",
               "descriptor pool handle is invalid");
  COMET_ASSERT(count > 0, "vulkan_descriptor_utils::AllocateDescriptor",
               "descriptor set count is zero");

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
      COMET_ASSERT(false, "vulkan_descriptor_utils::AllocateDescriptor",
                   "descriptor pool is fragmented");
      return false;

    case VK_ERROR_OUT_OF_POOL_MEMORY:
      COMET_ASSERT(false, "vulkan_descriptor_utils::AllocateDescriptor",
                   "descriptor pool is out of memory");
      return false;

    default:
      COMET_ASSERT(false, "vulkan_descriptor_utils::AllocateDescriptor",
                   "descriptor set allocation failed", "vk_result_value",
                   ToUnderlying(result));
      return false;
  }
}

void FreeDescriptor(VkDevice device_handle,
                    VkDescriptorSet descriptor_set_handle,
                    VkDescriptorPool& descriptor_pool_handle) {
  COMET_ASSERT(descriptor_set_handle != VK_NULL_HANDLE,
               "vulkan_descriptor_utils::FreeDescriptor",
               "descriptor set handle is invalid");

  return FreeDescriptor(device_handle, &descriptor_set_handle,
                        descriptor_pool_handle, 1);
}

void FreeDescriptor(VkDevice device_handle,
                    Array<VkDescriptorSet>& descriptor_set_handles,
                    VkDescriptorPool& descriptor_pool_handle) {
  COMET_ASSERT(!descriptor_set_handles.IsEmpty(),
               "vulkan_descriptor_utils::FreeDescriptor",
               "descriptor set handles are empty");

  return FreeDescriptor(device_handle, descriptor_set_handles.GetData(),
                        descriptor_pool_handle,
                        static_cast<u32>(descriptor_set_handles.GetSize()));
}

void FreeDescriptor(VkDevice device_handle,
                    VkDescriptorSet* descriptor_set_handles,
                    VkDescriptorPool& descriptor_pool_handle, u32 count) {
  COMET_ASSERT(device_handle != VK_NULL_HANDLE,
               "vulkan_descriptor_utils::FreeDescriptor",
               "device handle is invalid");
  COMET_ASSERT(descriptor_set_handles != nullptr,
               "vulkan_descriptor_utils::FreeDescriptor",
               "descriptor set handles are null");
  COMET_ASSERT(descriptor_pool_handle != VK_NULL_HANDLE,
               "vulkan_descriptor_utils::FreeDescriptor",
               "descriptor pool handle is invalid");
  COMET_ASSERT(count > 0, "vulkan_descriptor_utils::FreeDescriptor",
               "descriptor set count is zero");

  COMET_CHECK_VK(vkFreeDescriptorSets(device_handle, descriptor_pool_handle,
                                      count, descriptor_set_handles),
                 "vulkan_descriptor_utils::FreeDescriptor",
                 "descriptor set free failed");

  for (u32 i{0}; i < count; ++i) {
    descriptor_set_handles[i] = VK_NULL_HANDLE;
  }
}

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
void SetDescriptorSetLabels(const VkDescriptorSet* set_handles, u32 count,
                            const schar* prefix) {
  COMET_ASSERT(set_handles != nullptr,
               "vulkan_descriptor_utils::SetDescriptorSetLabels",
               "descriptor set handles are null");
  COMET_ASSERT(count > 0, "vulkan_descriptor_utils::SetDescriptorSetLabels",
               "descriptor set count is zero");
  COMET_ASSERT(prefix != nullptr,
               "vulkan_descriptor_utils::SetDescriptorSetLabels",
               "label prefix is null");

  constexpr usize kMaxPrefixLen{32};
  const auto prefix_len{math::Min(kMaxPrefixLen, GetLength(prefix))};
  constexpr auto kBufferLen{kMaxPrefixLen + GetCharCount<u32>() + 1};
  schar buffer[kBufferLen]{};
  memory::CopyMemory(buffer, prefix, prefix_len);

  for (u32 i{0}; i < count; ++i) {
    ConvertToStr(i, buffer + prefix_len, kBufferLen);
    COMET_VK_SET_DEBUG_LABEL(set_handles[i], buffer);
  }
}
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
}  // namespace vk
}  // namespace rendering
}  // namespace comet