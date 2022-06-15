// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_initializers.h"

namespace comet {
namespace rendering {
namespace vk {
namespace init {
VkDebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerCreateInfo(
    PFN_vkDebugUtilsMessengerCallbackEXT callback) {
  VkDebugUtilsMessengerCreateInfoEXT info{};
  info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  info.pfnUserCallback = callback;
  info.pUserData = VK_NULL_HANDLE;

  return info;
}

VkCommandPoolCreateInfo GetCommandPoolCreateInfo(
    u32 queue_family_index, VkCommandPoolCreateFlags flags) {
  VkCommandPoolCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.flags = flags;

  return info;
}

VkCommandBufferAllocateInfo GetCommandBufferAllocateInfo(
    VkCommandPool pool, u32 count, VkCommandBufferLevel level) {
  VkCommandBufferAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.commandPool = pool;
  info.commandBufferCount = count;
  info.level = level;

  return info;
}

VkCommandBufferBeginInfo GetCommandBufferBeginInfo(
    VkCommandBufferUsageFlags flags) {
  VkCommandBufferBeginInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.pInheritanceInfo = VK_NULL_HANDLE;
  info.flags = flags;

  return info;
}

VkDeviceQueueCreateInfo GetDeviceQueueCreateInfo(u32 queue_family_index,
                                                 const f32& queue_priority) {
  VkDeviceQueueCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  info.queueFamilyIndex = queue_family_index;
  info.queueCount = 1;
  info.pQueuePriorities = &queue_priority;

  return info;
}

VkDeviceCreateInfo GetDeviceCreateInfo(
    const std::vector<VkDeviceQueueCreateInfo>& queue_create_info,
    const VkPhysicalDeviceFeatures& physical_device_features,
    const std::vector<const char*>& device_extensions) {
  VkDeviceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  info.queueCreateInfoCount = static_cast<u32>(queue_create_info.size());
  info.pQueueCreateInfos = queue_create_info.data();
  info.pEnabledFeatures = &physical_device_features;

  info.enabledExtensionCount = static_cast<u32>(device_extensions.size());
  info.ppEnabledExtensionNames = device_extensions.data();

  return info;
}

VkFramebufferCreateInfo GetFrameBufferCreateInfo(VkRenderPass render_pass,
                                                 VkExtent2D extent) {
  VkFramebufferCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.renderPass = render_pass;
  info.attachmentCount = 1;
  info.width = extent.width;
  info.layers = 1;

  return info;
}

VkSwapchainCreateInfoKHR GetSwapchainCreateInfo(
    const VkSurfaceKHR& surface, const VkSurfaceFormatKHR& surface_format,
    const VkExtent2D& extent, const VkPresentModeKHR& present_mode,
    const SwapchainSupportDetails& details,
    const std::vector<u32>& queue_family_unique_indices, u32 image_count) {
  VkSwapchainCreateInfoKHR info{};
  info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  info.surface = surface;
  info.minImageCount = image_count;
  info.imageFormat = surface_format.format;
  info.imageColorSpace = surface_format.colorSpace;
  info.imageExtent = extent;
  info.imageArrayLayers = 1;  // For 2D purposes.
  info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if (queue_family_unique_indices.size() == 1) {
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 0;             // Optional.
    info.pQueueFamilyIndices = VK_NULL_HANDLE;  // Optional.
  } else {
    info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    info.queueFamilyIndexCount =
        static_cast<u32>(queue_family_unique_indices.size());
    info.pQueueFamilyIndices = queue_family_unique_indices.data();
  }

  info.preTransform = details.capabilities.currentTransform;
  info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  info.presentMode = present_mode;
  info.clipped = VK_TRUE;

  // TODO(m4jr0): Handle old swapchain better.
  info.oldSwapchain = VK_NULL_HANDLE;

  return info;
}

VkFenceCreateInfo GetFenceCreateInfo(VkFenceCreateFlags flags) {
  VkFenceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.flags = flags;

  return info;
}

VkSemaphoreCreateInfo GetSemaphoreCreateInfo(VkSemaphoreCreateFlags flags) {
  VkSemaphoreCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.flags = flags;

  return info;
}

VkSubmitInfo GetSubmitInfo(VkCommandBuffer* command_buffer) {
  VkSubmitInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.waitSemaphoreCount = 0;
  info.pWaitSemaphores = VK_NULL_HANDLE;
  info.pWaitDstStageMask = VK_NULL_HANDLE;
  info.commandBufferCount = 1;
  info.pCommandBuffers = command_buffer;
  info.signalSemaphoreCount = 0;
  info.pSignalSemaphores = VK_NULL_HANDLE;

  return info;
}

VkPresentInfoKHR GetPresentInfo() {
  VkPresentInfoKHR info{};
  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.pNext = VK_NULL_HANDLE;
  info.swapchainCount = 0;
  info.pSwapchains = VK_NULL_HANDLE;
  info.pWaitSemaphores = VK_NULL_HANDLE;
  info.waitSemaphoreCount = 0;
  info.pImageIndices = VK_NULL_HANDLE;

  return info;
}

VkRenderPassBeginInfo GetRenderPassBeginInfo(VkRenderPass render_pass,
                                             VkExtent2D extent,
                                             VkFramebuffer frame_buffer) {
  VkRenderPassBeginInfo info{};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.renderPass = render_pass;
  info.renderArea.offset.x = 0;
  info.renderArea.offset.y = 0;
  info.renderArea.extent = extent;
  info.clearValueCount = 1;
  info.pClearValues = VK_NULL_HANDLE;
  info.framebuffer = frame_buffer;

  return info;
}

VkPipelineShaderStageCreateInfo GetPipelineShaderStageCreateInfo(
    VkShaderStageFlagBits stage, VkShaderModule shader_module) {
  VkPipelineShaderStageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.stage = stage;
  info.module = shader_module;
  info.pName = "main";

  return info;
}

VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputStateCreateInfo(
    const VkVertexInputBindingDescription* binding_descriptions,
    u32 binding_description_count,
    const VkVertexInputAttributeDescription* attribute_descriptions,
    u32 attribute_description_count) {
  VkPipelineVertexInputStateCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.pVertexBindingDescriptions = binding_descriptions;
  info.vertexBindingDescriptionCount = binding_description_count;
  info.pVertexAttributeDescriptions = attribute_descriptions;
  info.vertexAttributeDescriptionCount = attribute_description_count;

  return info;
}

VkPipelineInputAssemblyStateCreateInfo GetPipelineInputAssemblyStateCreateInfo(
    VkPrimitiveTopology topology, bool is_primitive_restart_enabled) {
  VkPipelineInputAssemblyStateCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.topology = topology;
  info.primitiveRestartEnable = is_primitive_restart_enabled;

  return info;
}

VkPipelineRasterizationStateCreateInfo GetPipelineRasterizationStateCreateInfo(
    VkPolygonMode polygon_mode) {
  VkPipelineRasterizationStateCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.depthClampEnable = VK_FALSE;
  info.rasterizerDiscardEnable = VK_FALSE;
  info.polygonMode = polygon_mode;
  info.lineWidth = 1.0f;
  info.cullMode = VK_CULL_MODE_NONE;
  info.frontFace = VK_FRONT_FACE_CLOCKWISE;
  info.depthBiasEnable = VK_FALSE;
  info.depthBiasConstantFactor = 0.0f;
  info.depthBiasClamp = 0.0f;
  info.depthBiasSlopeFactor = 0.0f;

  return info;
}

VkPipelineMultisampleStateCreateInfo GetPipelineMultisampleStateCreateInfo() {
  VkPipelineMultisampleStateCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.sampleShadingEnable = VK_FALSE;
  info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  info.minSampleShading = 1.0f;
  info.pSampleMask = VK_NULL_HANDLE;
  info.alphaToCoverageEnable = VK_FALSE;
  info.alphaToOneEnable = VK_FALSE;

  return info;
}

VkPipelineColorBlendAttachmentState GetPipelineColorBlendAttachmentState() {
  VkPipelineColorBlendAttachmentState attachment{};
  attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  attachment.blendEnable = VK_FALSE;
  attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  attachment.colorBlendOp = VK_BLEND_OP_ADD;
  attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  attachment.alphaBlendOp = VK_BLEND_OP_ADD;

  return attachment;
}

VkPipelineColorBlendStateCreateInfo GetPipelineColorBlendStateCreateInfo(
    const VkPipelineColorBlendAttachmentState* color_blend_attachments,
    uindex color_blend_attachment_count) {
  VkPipelineColorBlendStateCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  info.logicOpEnable = VK_FALSE;
  info.logicOp = VK_LOGIC_OP_COPY;
  info.attachmentCount = static_cast<u32>(color_blend_attachment_count);
  info.pAttachments = color_blend_attachments;
  info.blendConstants[0] = 0.0f;
  info.blendConstants[1] = 0.0f;
  info.blendConstants[2] = 0.0f;
  info.blendConstants[3] = 0.0f;
  info.pNext = VK_NULL_HANDLE;

  return info;
}

VkPipelineLayoutCreateInfo GetPipelineLayoutCreateInfo() {
  VkPipelineLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.flags = 0;
  info.setLayoutCount = 0;
  info.pSetLayouts = VK_NULL_HANDLE;
  info.pushConstantRangeCount = 0;
  info.pPushConstantRanges = VK_NULL_HANDLE;

  return info;
}

VkPipelineDepthStencilStateCreateInfo GetPipelineDepthStencilStateCreateInfo(
    bool is_depth_test, bool is_depth_write, VkCompareOp compare_op) {
  VkPipelineDepthStencilStateCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.depthTestEnable = is_depth_test;
  info.depthWriteEnable = is_depth_write;
  info.depthCompareOp = is_depth_test ? compare_op : VK_COMPARE_OP_ALWAYS;
  info.depthBoundsTestEnable = VK_FALSE;
  info.minDepthBounds = 0.0f;
  info.maxDepthBounds = 1.0f;
  info.stencilTestEnable = VK_FALSE;
  info.front = {};
  info.back = {};

  return info;
}

VkImageCreateInfo GetImageCreateInfo(u32 width, u32 height, u32 mip_levels,
                                     VkSampleCountFlagBits num_samples,
                                     VkFormat format, VkImageTiling tiling,
                                     VkImageUsageFlags usage_flags,
                                     VkSharingMode sharing_mode,
                                     const u32* queue_family_indices,
                                     u32 queue_family_index_count) {
  VkImageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.imageType = VK_IMAGE_TYPE_2D;
  info.extent.width = width;
  info.extent.height = height;
  info.extent.depth = 1;
  info.mipLevels = mip_levels;
  info.arrayLayers = 1;
  info.format = format;
  info.tiling = tiling;
  info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  info.usage = usage_flags;
  info.samples = num_samples;
  info.flags = 0;  // Optional.
  info.sharingMode = sharing_mode;
  info.queueFamilyIndexCount = queue_family_index_count;
  info.pQueueFamilyIndices = queue_family_indices;

  return info;
}

VkImageViewCreateInfo GetImageViewCreateInfo(VkImage image, VkFormat format,
                                             VkImageAspectFlags aspect_flags,
                                             u32 mip_levels) {
  VkImageViewCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  info.image = image;
  info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  info.format = format;

  info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  info.subresourceRange.aspectMask = aspect_flags;
  info.subresourceRange.baseMipLevel = 0;
  info.subresourceRange.levelCount = mip_levels;
  info.subresourceRange.baseArrayLayer = 0;
  info.subresourceRange.layerCount = 1;

  return info;
}

VkDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding(
    VkDescriptorType type, VkShaderStageFlags stage_flags, u32 binding) {
  VkDescriptorSetLayoutBinding layout_binding{};
  layout_binding.binding = binding;
  layout_binding.descriptorCount = 1;
  layout_binding.descriptorType = type;
  layout_binding.pImmutableSamplers = VK_NULL_HANDLE;
  layout_binding.stageFlags = stage_flags;

  return layout_binding;
}

VkWriteDescriptorSet GetBufferWriteDescriptorSet(
    VkDescriptorType type, VkDescriptorSet dst_set,
    VkDescriptorBufferInfo* buffer_info, u32 binding) {
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = VK_NULL_HANDLE;
  write.dstBinding = binding;
  write.dstSet = dst_set;
  write.descriptorCount = 1;
  write.descriptorType = type;
  write.pBufferInfo = buffer_info;

  return write;
}

VkWriteDescriptorSet GetImageWriteDescriptorSet(
    VkDescriptorType type, VkDescriptorSet dst_set,
    VkDescriptorImageInfo* image_info, u32 binding) {
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = VK_NULL_HANDLE;
  write.dstBinding = binding;
  write.dstSet = dst_set;
  write.descriptorCount = 1;
  write.descriptorType = type;
  write.pImageInfo = image_info;

  return write;
}

VkSamplerCreateInfo GetSamplerCreateInfo(VkFilter filters,
                                         VkSamplerAddressMode address_mode) {
  VkSamplerCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.magFilter = filters;
  info.minFilter = filters;
  info.addressModeU = address_mode;
  info.addressModeV = address_mode;
  info.addressModeW = address_mode;

  return info;
}
}  // namespace init
}  // namespace vk
}  // namespace rendering
}  // namespace comet
