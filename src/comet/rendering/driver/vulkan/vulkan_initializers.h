// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_INITIALIZERS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_INITIALIZERS_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/vulkan_types.h"

namespace comet {
namespace rendering {
namespace vk {
namespace init {
VkDebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerCreateInfo(
    PFN_vkDebugUtilsMessengerCallbackEXT callback);
VkCommandPoolCreateInfo GetCommandPoolCreateInfo(
    u32 queue_family_index, VkCommandPoolCreateFlags flags = 0);
VkCommandBufferAllocateInfo GetCommandBufferAllocateInfo(
    VkCommandPool pool, u32 count = 1,
    VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
VkCommandBufferBeginInfo GetCommandBufferBeginInfo(
    VkCommandBufferUsageFlags flags = 0);
VkFramebufferCreateInfo GetFrameBufferCreateInfo(VkRenderPass render_pass,
                                                 VkExtent2D extent);
VkDeviceQueueCreateInfo GetDeviceQueueCreateInfo(u32 queue_family_index,
                                                 const f32& queue_priority);
VkDeviceCreateInfo GetDeviceCreateInfo(
    const std::vector<VkDeviceQueueCreateInfo>& queue_create_info,
    const VkPhysicalDeviceFeatures& physical_device_features,
    const std::vector<const char*>& device_extensions);
VkSwapchainCreateInfoKHR GetSwapchainCreateInfo(
    const VkSurfaceKHR& surface, const VkSurfaceFormatKHR& surface_format,
    const VkExtent2D& extent, const VkPresentModeKHR& present_mode,
    const SwapChainSupportDetails& details,
    const std::vector<u32>& queue_family_unique_indices, u32 image_count);
VkFenceCreateInfo GetFenceCreateInfo(VkFenceCreateFlags flags = 0);
VkSemaphoreCreateInfo GetSemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);
VkSubmitInfo GetSubmitInfo(VkCommandBuffer* command_buffer);
VkPresentInfoKHR GetPresentInfo();
VkRenderPassBeginInfo GetRenderPassBeginInfo(VkRenderPass render_pass,
                                             VkExtent2D extent,
                                             VkFramebuffer frame_buffer);
VkPipelineShaderStageCreateInfo GetPipelineShaderStageCreateInfo(
    VkShaderStageFlagBits stage, VkShaderModule shader_module);
VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputStateCreateInfo();
VkPipelineInputAssemblyStateCreateInfo GetPipelineInputAssemblyStateCreateInfo(
    VkPrimitiveTopology topology);
VkPipelineRasterizationStateCreateInfo GetPipelineRasterizationStateCreateInfo(
    VkPolygonMode polygon_mode);
VkPipelineMultisampleStateCreateInfo GetPipelineMultisampleStateCreateInfo();
VkPipelineColorBlendAttachmentState GetPipelineColorBlendAttachmentState();
VkPipelineLayoutCreateInfo GetPipelineLayoutCreateInfo();
VkPipelineDepthStencilStateCreateInfo GetPipelineDepthStencilStateCreateInfo(
    bool is_depth_test, bool is_depth_write, VkCompareOp compare_op);
VkImageCreateInfo GetImageCreateInfo(
    u32 width, u32 height, u32 mip_levels, VkSampleCountFlagBits num_samples,
    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage_flags,
    VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
    const u32* queue_family_indices = nullptr,
    u32 queue_family_index_count = 0);
VkImageViewCreateInfo GetImageViewCreateInfo(VkImage image, VkFormat format,
                                             VkImageAspectFlags aspect_flags,
                                             u32 mip_levels);
VkDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding(
    VkDescriptorType type, VkShaderStageFlags stage_flags, u32 binding);
VkWriteDescriptorSet GetBufferWriteDescriptorSet(
    VkDescriptorType type, VkDescriptorSet dst_set,
    VkDescriptorBufferInfo* buffer_info, u32 binding);
VkWriteDescriptorSet GetImageWriteDescriptorSet(
    VkDescriptorType type, VkDescriptorSet dst_set,
    VkDescriptorImageInfo* image_info, u32 binding);
VkSamplerCreateInfo GetSamplerCreateInfo(
    VkFilter filters,
    VkSamplerAddressMode address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
}  // namespace init
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_INITIALIZERS_H_
