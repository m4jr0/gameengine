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
    std::uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);
VkCommandBufferAllocateInfo GetCommandBufferAllocateInfo(
    VkCommandPool pool, std::uint32_t count = 1,
    VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
VkCommandBufferBeginInfo GetCommandBufferBeginInfo(
    VkCommandBufferUsageFlags flags = 0);
VkFramebufferCreateInfo GetFrameBufferCreateInfo(VkRenderPass render_pass,
                                                 VkExtent2D extent);
VkDeviceQueueCreateInfo GetDeviceQueueCreateInfo(
    std::uint32_t queue_family_index, const float& queue_priority);
VkDeviceCreateInfo GetDeviceCreateInfo(
    const std::vector<VkDeviceQueueCreateInfo>& queue_create_info,
    const VkPhysicalDeviceFeatures& physical_device_features,
    const std::vector<const char*>& device_extensions);
VkSwapchainCreateInfoKHR GetSwapchainCreateInfo(
    const VkSurfaceKHR& surface, const VkSurfaceFormatKHR& surface_format,
    const VkExtent2D& extent, const VkPresentModeKHR& present_mode,
    const SwapChainSupportDetails& details,
    const std::vector<std::uint32_t>& queue_family_unique_indices,
    std::uint32_t image_count);
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
    std::uint32_t width, std::uint32_t height, std::uint32_t mip_levels,
    VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage_flags,
    VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
    const std::uint32_t* queue_family_indices = nullptr,
    std::uint32_t queue_family_index_count = 0);
VkImageViewCreateInfo GetImageViewCreateInfo(VkImage image, VkFormat format,
                                             VkImageAspectFlags aspect_flags,
                                             std::uint32_t mip_levels);
VkDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding(
    VkDescriptorType type, VkShaderStageFlags stage_flags,
    std::uint32_t binding);
VkWriteDescriptorSet GetBufferWriteDescriptorSet(
    VkDescriptorType type, VkDescriptorSet dst_set,
    VkDescriptorBufferInfo* buffer_info, std::uint32_t binding);
VkWriteDescriptorSet GetImageWriteDescriptorSet(
    VkDescriptorType type, VkDescriptorSet dst_set,
    VkDescriptorImageInfo* image_info, std::uint32_t binding);
VkSamplerCreateInfo GetSamplerCreateInfo(
    VkFilter filters,
    VkSamplerAddressMode address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
}  // namespace init
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_INITIALIZERS_H_
