// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_INITIALIZER_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_INITIALIZER_UTILS_H_

#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/data/vulkan_pipeline.h"
#include "comet/rendering/driver/vulkan/vulkan_swapchain.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/material_resource.h"

namespace comet {
namespace rendering {
namespace vk {
namespace init {
VkDebugUtilsMessengerCreateInfoEXT GenerateDebugUtilsMessengerCreateInfo(
    PFN_vkDebugUtilsMessengerCallbackEXT callback);
VkCommandPoolCreateInfo GenerateCommandPoolCreateInfo(
    u32 queue_family_index, VkCommandPoolCreateFlags flags = 0);
VkCommandBufferAllocateInfo GenerateCommandBufferAllocateInfo(
    VkCommandPool command_pool_handle, u32 count = 1,
    VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
VkCommandBufferBeginInfo GenerateCommandBufferBeginInfo(
    VkCommandBufferUsageFlags flags = 0);
VkFramebufferCreateInfo GenerateFrameBufferCreateInfo(
    VkRenderPass render_pass_handle, VkExtent2D extent);
VkDeviceQueueCreateInfo GenerateDeviceQueueCreateInfo(
    u32 queue_family_index, const f32& queue_priority);
VkDeviceCreateInfo GenerateDeviceCreateInfo(
    const Array<VkDeviceQueueCreateInfo>& queue_create_info,
    const VkPhysicalDeviceFeatures& physical_device_features,
    const schar* const* device_extensions, u32 device_extension_count,
    const void* next = VK_NULL_HANDLE);
VkSwapchainCreateInfoKHR GenerateSwapchainCreateInfo(
    VkSurfaceKHR surface_handle, const VkSurfaceFormatKHR& surface_format,
    const VkExtent2D& extent, const VkPresentModeKHR& present_mode,
    const SwapchainSupportDetails& details,
    const Array<u32>& queue_family_unique_indices, u32 image_count);
VkFenceCreateInfo GenerateFenceCreateInfo(VkFenceCreateFlags flags = 0);
VkSemaphoreCreateInfo GenerateSemaphoreCreateInfo(
    VkSemaphoreCreateFlags flags = 0);
VkSubmitInfo GenerateSubmitInfo(
    const VkCommandBuffer* command_buffer_handle,
    const VkSemaphore* wait_semaphores, u32 wait_semaphore_count,
    const VkSemaphore* signal_semaphores, u32 signal_semaphore_count,
    const VkPipelineStageFlags* wait_dst_stage_mask = VK_NULL_HANDLE,
    const void* next = VK_NULL_HANDLE);
VkSubmitInfo2 GenerateSubmitInfo2(
    u32 command_buffer_info_count,
    const VkCommandBufferSubmitInfo* command_buffer_infos,
    const VkSemaphoreSubmitInfo* wait_semaphore_infos, u32 wait_semaphore_count,
    const VkSemaphoreSubmitInfo* signal_semaphore_infos,
    u32 signal_semaphore_info_count, const void* next = VK_NULL_HANDLE);
VkPresentInfoKHR GeneratePresentInfo();
VkRenderPassBeginInfo GenerateRenderPassBeginInfo(
    VkRenderPass render_pass_handle, VkExtent2D extent,
    VkFramebuffer framebuffer_handle);
VkPipelineShaderStageCreateInfo GeneratePipelineShaderStageCreateInfo(
    VkShaderStageFlagBits stage, VkShaderModule shader_module_handle);
VkPipelineVertexInputStateCreateInfo GeneratePipelineVertexInputStateCreateInfo(
    const VkVertexInputBindingDescription* binding_descriptions = nullptr,
    u32 binding_description_count = 0,
    const VkVertexInputAttributeDescription* vertex_attribute_descriptions =
        nullptr,
    u32 vertex_attribute_description_count = 0);
VkPipelineInputAssemblyStateCreateInfo
GeneratePipelineInputAssemblyStateCreateInfo(
    PrimitiveTopology topology, bool is_primitive_restart_enabled = VK_FALSE);
VkPipelineRasterizationStateCreateInfo
GeneratePipelineRasterizationStateCreateInfo(
    bool is_wireframe, VkCullModeFlags cull_mode = VK_CULL_MODE_NONE);
VkPipelineRasterizationStateCreateInfo
GeneratePipelineRasterizationStateCreateInfo(bool is_wireframe,
                                             CullMode = CullMode::None);
VkPipelineMultisampleStateCreateInfo
GeneratePipelineMultisampleStateCreateInfo();
VkPipelineColorBlendAttachmentState GeneratePipelineColorBlendAttachmentState();
VkPipelineColorBlendStateCreateInfo GeneratePipelineColorBlendStateCreateInfo(
    const VkPipelineColorBlendAttachmentState* color_blend_attachments,
    usize color_blend_attachment_count);
VkPipelineLayoutCreateInfo GeneratePipelineLayoutCreateInfo(
    const PipelineLayoutDescr& descr);
VkPipelineDepthStencilStateCreateInfo
GeneratePipelineDepthStencilStateCreateInfo(
    bool is_depth_test, bool is_depth_write,
    VkCompareOp compare_op = VK_COMPARE_OP_ALWAYS);
VkImageCreateInfo GenerateImageCreateInfo(
    u32 width, u32 height, u32 mip_levels, VkSampleCountFlagBits num_samples,
    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage_flags,
    VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
    const u32* queue_family_indices = nullptr,
    u32 queue_family_index_count = 0);
VkImageViewCreateInfo GenerateImageViewCreateInfo(
    VkImage image_handle, VkFormat format, VkImageAspectFlags aspect_flags,
    u32 mip_levels);
VkDescriptorSetLayoutCreateInfo GenerateDescriptorSetLayoutCreateInfo(
    const DescriptorSetLayoutBinding& descriptor_set_data);
VkDescriptorSetLayoutBinding GenerateDescriptorSetLayoutBinding(
    VkDescriptorType type, VkShaderStageFlags stage_flags, u32 binding);
VkDescriptorBufferInfo GenerateDescriptorBufferInfo(VkBuffer buffer_handle,
                                                    sptrdiff offset,
                                                    usize stride);
VkDescriptorImageInfo GenerateDescriptorImageInfo(VkSampler sampler_handle,
                                                  VkImageView image_view_handle,
                                                  VkImageLayout image_layout);
VkWriteDescriptorSet GenerateBufferWriteDescriptorSet(
    VkDescriptorType type, VkDescriptorSet dst_descriptor_set_handle,
    const VkDescriptorBufferInfo* buffer_info, u32 binding);
VkWriteDescriptorSet GenerateImageWriteDescriptorSet(
    VkDescriptorType type, VkDescriptorSet dst_descriptor_set_handle,
    const VkDescriptorImageInfo* image_info, u32 binding);
VkWriteDescriptorSet GenerateImageWriteDescriptorSet(
    VkDescriptorType type, VkDescriptorSet dst_descriptor_set_handle,
    const VkDescriptorImageInfo* image_info_list, u32 image_info_count,
    u32 binding);
VkSamplerCreateInfo GenerateSamplerCreateInfo(
    VkFilter filters,
    VkSamplerAddressMode address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
VkSamplerCreateInfo GenerateSamplerCreateInfo(
    VkFilter filters, VkSamplerAddressMode address_mode,
    const resource::TextureMap* texture_map, bool is_sampler_anisotropy,
    f32 max_sampler_anisotropy);
VkBufferMemoryBarrier GenerateBufferMemoryBarrier(
    const Buffer& buffer, VkAccessFlags src_access_mask,
    VkAccessFlags dst_access_mask,
    u32 src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
    u32 dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
    VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
VkBufferMemoryBarrier GenerateBufferMemoryBarrier(
    VkBuffer buffer_handle, VkAccessFlags src_access_mask,
    VkAccessFlags dst_access_mask,
    u32 src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
    u32 dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
    VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
VkTimelineSemaphoreSubmitInfo GenerateTimelineSemaphoreSubmitInfo(
    u32 wait_semaphore_value_count, const u64* wait_semaphore_values,
    u32 signal_semaphore_value_count, const u64* signal_semaphore_values);
}  // namespace init
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_INITIALIZER_UTILS_H_
