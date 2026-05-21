// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/utils/vulkan_initializer_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type_trait.h"
#include "comet/rendering/driver/vulkan/type/vulkan_pipeline.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_sampler_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_shader_utils.h"
#include "comet/rendering/label/pipeline_label.h"

namespace comet {
namespace rendering {
namespace vk {
namespace init {
VkDebugUtilsMessengerCreateInfoEXT GenerateDebugUtilsMessengerCreateInfo(
    PFN_vkDebugUtilsMessengerCallbackEXT callback) {
  VkDebugUtilsMessengerCreateInfoEXT info{};
  info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  info.pfnUserCallback = callback;
  info.pUserData = VK_NULL_HANDLE;
  return info;
}

VkCommandPoolCreateInfo GenerateCommandPoolCreateInfo(
    u32 queue_family_index, VkCommandPoolCreateFlags flags) {
  VkCommandPoolCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.flags = flags;
  info.queueFamilyIndex = queue_family_index;
  return info;
}

VkCommandBufferAllocateInfo GenerateCommandBufferAllocateInfo(
    VkCommandPool command_pool_handle, u32 count, VkCommandBufferLevel level) {
  COMET_ASSERT(command_pool_handle != VK_NULL_HANDLE,
               "vulkan_initializer_utils::GenerateCommandBufferAllocateInfo",
               "command pool handle is invalid");
  COMET_ASSERT(count > 0,
               "vulkan_initializer_utils::GenerateCommandBufferAllocateInfo",
               "command buffer count is zero");

  VkCommandBufferAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.commandPool = command_pool_handle;
  info.commandBufferCount = count;
  info.level = level;
  return info;
}

VkCommandBufferBeginInfo GenerateCommandBufferBeginInfo(
    VkCommandBufferUsageFlags flags) {
  VkCommandBufferBeginInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.pInheritanceInfo = VK_NULL_HANDLE;
  info.flags = flags;
  return info;
}

VkDeviceQueueCreateInfo GenerateDeviceQueueCreateInfo(
    u32 queue_family_index, const f32& queue_priority) {
  COMET_ASSERT(queue_priority >= .0f && queue_priority <= 1.0f,
               "vulkan_initializer_utils::GenerateDeviceQueueCreateInfo",
               "queue priority is out of range", "queue_priority",
               queue_priority);

  VkDeviceQueueCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  info.queueFamilyIndex = queue_family_index;
  info.queueCount = 1;
  info.pQueuePriorities = &queue_priority;
  return info;
}

VkDeviceCreateInfo GenerateDeviceCreateInfo(
    const Array<VkDeviceQueueCreateInfo>& queue_create_info,
    const VkPhysicalDeviceFeatures& physical_device_features,
    const schar* const* device_extensions, u32 device_extension_count,
    const void* next) {
  VkDeviceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  info.pNext = next;
  info.queueCreateInfoCount = static_cast<u32>(queue_create_info.GetSize());
  info.pQueueCreateInfos = queue_create_info.GetData();
  info.pEnabledFeatures = &physical_device_features;
  info.enabledExtensionCount = device_extension_count;
  info.ppEnabledExtensionNames = device_extensions;
  return info;
}

VkFramebufferCreateInfo GenerateFrameBufferCreateInfo(
    VkRenderPass render_pass_handle, VkExtent2D extent) {
  COMET_ASSERT(render_pass_handle != VK_NULL_HANDLE,
               "vulkan_initializer_utils::GenerateFrameBufferCreateInfo",
               "render pass handle is invalid");
  COMET_ASSERT(extent.width > 0,
               "vulkan_initializer_utils::GenerateFrameBufferCreateInfo",
               "framebuffer width is zero");
  COMET_ASSERT(extent.height > 0,
               "vulkan_initializer_utils::GenerateFrameBufferCreateInfo",
               "framebuffer height is zero");

  VkFramebufferCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.renderPass = render_pass_handle;
  info.attachmentCount = 1;
  info.width = extent.width;
  info.height = extent.height;
  info.layers = 1;
  return info;
}

VkSwapchainCreateInfoKHR GenerateSwapchainCreateInfo(
    VkSurfaceKHR surface_handle, const VkSurfaceFormatKHR& surface_format,
    const VkExtent2D& extent, const VkPresentModeKHR& present_mode,
    const SwapchainSupportDetails& details,
    const Array<u32>& queue_family_unique_indices, u32 image_count) {
  COMET_ASSERT(surface_handle != VK_NULL_HANDLE,
               "vulkan_initializer_utils::GenerateSwapchainCreateInfo",
               "surface handle is invalid");
  COMET_ASSERT(image_count > 0,
               "vulkan_initializer_utils::GenerateSwapchainCreateInfo",
               "swapchain image count is zero");
  COMET_ASSERT(extent.width > 0,
               "vulkan_initializer_utils::GenerateSwapchainCreateInfo",
               "swapchain width is zero");
  COMET_ASSERT(extent.height > 0,
               "vulkan_initializer_utils::GenerateSwapchainCreateInfo",
               "swapchain height is zero");
  COMET_ASSERT(!queue_family_unique_indices.IsEmpty(),
               "vulkan_initializer_utils::GenerateSwapchainCreateInfo",
               "queue family indices are empty");

  VkSwapchainCreateInfoKHR info{};
  info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  info.surface = surface_handle;
  info.minImageCount = image_count;
  info.imageFormat = surface_format.format;
  info.imageColorSpace = surface_format.colorSpace;
  info.imageExtent = extent;
  info.imageArrayLayers = 1;  // For 2D purposes.
  info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if (queue_family_unique_indices.GetSize() == 1) {
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 0;             // Optional.
    info.pQueueFamilyIndices = VK_NULL_HANDLE;  // Optional.
  } else {
    info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    info.queueFamilyIndexCount =
        static_cast<u32>(queue_family_unique_indices.GetSize());
    info.pQueueFamilyIndices = queue_family_unique_indices.GetData();
  }

  info.preTransform = details.capabilities.currentTransform;
  info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  info.presentMode = present_mode;
  info.clipped = VK_TRUE;

  // TODO(m4jr0): Handle old swapchain better.
  info.oldSwapchain = VK_NULL_HANDLE;
  return info;
}

VkFenceCreateInfo GenerateFenceCreateInfo(VkFenceCreateFlags flags) {
  VkFenceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.flags = flags;
  return info;
}

VkSemaphoreCreateInfo GenerateSemaphoreCreateInfo(
    VkSemaphoreCreateFlags flags) {
  VkSemaphoreCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.flags = flags;
  return info;
}

VkSubmitInfo GenerateSubmitInfo(const VkCommandBuffer* command_buffer_handle,
                                const VkSemaphore* wait_semaphores,
                                u32 wait_semaphore_count,
                                const VkSemaphore* signal_semaphores,
                                u32 signal_semaphore_count,
                                const VkPipelineStageFlags* wait_dst_stage_mask,
                                const void* next) {
  COMET_ASSERT(command_buffer_handle != nullptr,
               "vulkan_initializer_utils::GenerateSubmitInfo",
               "command buffer handle pointer is null");
  COMET_ASSERT(wait_semaphore_count == 0 || wait_semaphores != nullptr,
               "vulkan_initializer_utils::GenerateSubmitInfo",
               "wait semaphores are null");
  COMET_ASSERT(wait_semaphore_count == 0 || wait_dst_stage_mask != nullptr,
               "vulkan_initializer_utils::GenerateSubmitInfo",
               "wait stage masks are null");
  COMET_ASSERT(signal_semaphore_count == 0 || signal_semaphores != nullptr,
               "vulkan_initializer_utils::GenerateSubmitInfo",
               "signal semaphores are null");

  VkSubmitInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  info.pNext = next;
  info.waitSemaphoreCount = wait_semaphore_count;
  info.pWaitSemaphores = wait_semaphores;
  info.pWaitDstStageMask = wait_dst_stage_mask;
  info.commandBufferCount = 1;
  info.pCommandBuffers = command_buffer_handle;
  info.signalSemaphoreCount = signal_semaphore_count;
  info.pSignalSemaphores = signal_semaphores;
  return info;
}

VkSubmitInfo2 GenerateSubmitInfo2(
    u32 command_buffer_info_count,
    const VkCommandBufferSubmitInfo* command_buffer_infos,
    const VkSemaphoreSubmitInfo* wait_semaphore_infos, u32 wait_semaphore_count,
    const VkSemaphoreSubmitInfo* signal_semaphore_infos,
    u32 signal_semaphore_info_count, const void* next) {
  COMET_ASSERT(command_buffer_info_count > 0,
               "vulkan_initializer_utils::GenerateSubmitInfo2",
               "command buffer submit info count is zero");
  COMET_ASSERT(command_buffer_infos != nullptr,
               "vulkan_initializer_utils::GenerateSubmitInfo2",
               "command buffer submit infos are null");
  COMET_ASSERT(wait_semaphore_count == 0 || wait_semaphore_infos != nullptr,
               "vulkan_initializer_utils::GenerateSubmitInfo2",
               "wait semaphore infos are null");
  COMET_ASSERT(
      signal_semaphore_info_count == 0 || signal_semaphore_infos != nullptr,
      "vulkan_initializer_utils::GenerateSubmitInfo2",
      "signal semaphore infos are null");

  VkSubmitInfo2 info{};
  info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  info.pNext = next;
  info.waitSemaphoreInfoCount = wait_semaphore_count;
  info.pWaitSemaphoreInfos = wait_semaphore_infos;
  info.commandBufferInfoCount = command_buffer_info_count;
  info.pCommandBufferInfos = command_buffer_infos;
  info.signalSemaphoreInfoCount = signal_semaphore_info_count;
  info.pSignalSemaphoreInfos = signal_semaphore_infos;
  return info;
}

VkPresentInfoKHR GeneratePresentInfo() {
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

VkRenderPassBeginInfo GenerateRenderPassBeginInfo(
    VkRenderPass render_pass_handle, VkExtent2D extent,
    VkFramebuffer framebuffer_handle) {
  COMET_ASSERT(render_pass_handle != VK_NULL_HANDLE,
               "vulkan_initializer_utils::GenerateRenderPassBeginInfo",
               "render pass handle is invalid");
  COMET_ASSERT(framebuffer_handle != VK_NULL_HANDLE,
               "vulkan_initializer_utils::GenerateRenderPassBeginInfo",
               "framebuffer handle is invalid");
  COMET_ASSERT(extent.width > 0,
               "vulkan_initializer_utils::GenerateRenderPassBeginInfo",
               "render pass width is zero");
  COMET_ASSERT(extent.height > 0,
               "vulkan_initializer_utils::GenerateRenderPassBeginInfo",
               "render pass height is zero");

  VkRenderPassBeginInfo info{};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.renderPass = render_pass_handle;
  info.renderArea.offset.x = 0;
  info.renderArea.offset.y = 0;
  info.renderArea.extent = extent;
  info.clearValueCount = 1;
  info.pClearValues = VK_NULL_HANDLE;
  info.framebuffer = framebuffer_handle;
  return info;
}

VkPipelineShaderStageCreateInfo GeneratePipelineShaderStageCreateInfo(
    VkShaderStageFlagBits stage, VkShaderModule shader_module_handle) {
  COMET_ASSERT(
      shader_module_handle != VK_NULL_HANDLE,
      "vulkan_initializer_utils::GeneratePipelineShaderStageCreateInfo",
      "shader module handle is invalid");

  VkPipelineShaderStageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.stage = stage;
  info.module = shader_module_handle;
  info.pName = "main";
  return info;
}

VkPipelineVertexInputStateCreateInfo GeneratePipelineVertexInputStateCreateInfo(
    const VkVertexInputBindingDescription* binding_descriptions,
    u32 binding_description_count,
    const VkVertexInputAttributeDescription* vertex_attribute_descriptions,
    u32 vertex_attribute_description_count) {
  VkPipelineVertexInputStateCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.pVertexBindingDescriptions = binding_descriptions;
  info.vertexBindingDescriptionCount = binding_description_count;
  info.pVertexAttributeDescriptions = vertex_attribute_descriptions;
  info.vertexAttributeDescriptionCount = vertex_attribute_description_count;
  return info;
}

VkPipelineInputAssemblyStateCreateInfo
GeneratePipelineInputAssemblyStateCreateInfo(
    PrimitiveTopology topology, bool is_primitive_restart_enabled) {
  VkPipelineInputAssemblyStateCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.primitiveRestartEnable = is_primitive_restart_enabled;

  auto vk_topology{VK_PRIMITIVE_TOPOLOGY_MAX_ENUM};

  switch (topology) {
    case PrimitiveTopology::Points:
      vk_topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
      break;
    case PrimitiveTopology::Lines:
      vk_topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      break;
    case PrimitiveTopology::LineStrip:
      vk_topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
      break;
    case PrimitiveTopology::Triangles:
      vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      break;
    case PrimitiveTopology::TriangleStrip:
      vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
      break;
    default:
      COMET_ASSERT(false,
                   "vulkan_initializer_utils::"
                   "GeneratePipelineInputAssemblyStateCreateInfo",
                   "primitive topology is unsupported", "topology",
                   GetPrimitiveTopologyLabel(topology), "topology_value",
                   ToUnderlying(topology));
  }

  info.topology = vk_topology;
  return info;
}

VkPipelineRasterizationStateCreateInfo
GeneratePipelineRasterizationStateCreateInfo(bool is_wireframe,
                                             VkCullModeFlags cull_mode,
                                             bool is_depth_bias) {
  VkPipelineRasterizationStateCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.depthClampEnable = VK_FALSE;
  info.rasterizerDiscardEnable = VK_FALSE;
  info.polygonMode = is_wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
  info.lineWidth = 1.0f;
  info.cullMode = cull_mode;
  info.frontFace = VK_FRONT_FACE_CLOCKWISE;
  info.depthBiasEnable = is_depth_bias ? VK_TRUE : VK_FALSE;
  info.depthBiasConstantFactor = .0f;
  info.depthBiasClamp = .0f;
  info.depthBiasSlopeFactor = .0f;
  return info;
}

VkPipelineRasterizationStateCreateInfo
GeneratePipelineRasterizationStateCreateInfo(bool is_wireframe,
                                             CullMode cull_mode,
                                             bool is_depth_bias) {
  VkCullModeFlags vk_cull_mode{VK_CULL_MODE_NONE};

  switch (cull_mode) {
    case CullMode::None:
      vk_cull_mode = VK_CULL_MODE_NONE;
      break;
    case CullMode::Front:
      vk_cull_mode = VK_CULL_MODE_FRONT_BIT;
      break;
    case CullMode::Back:
      vk_cull_mode = VK_CULL_MODE_BACK_BIT;
      break;
    case CullMode::FrontAndBack:
      vk_cull_mode = VK_CULL_MODE_FRONT_AND_BACK;
      break;
    default:
      COMET_ASSERT(false,
                   "vulkan_initializer_utils::"
                   "GeneratePipelineRasterizationStateCreateInfo",
                   "cull mode is unsupported", "cull_mode",
                   GetCullModeLabel(cull_mode), "cull_mode_value",
                   ToUnderlying(cull_mode));
  }

  return GeneratePipelineRasterizationStateCreateInfo(
      is_wireframe, vk_cull_mode, is_depth_bias);
}

VkPipelineRasterizationStateCreateInfo
GeneratePipelineRasterizationStateCreateInfo(
    const RasterizerState& rasterizer) {
  return GeneratePipelineRasterizationStateCreateInfo(
      rasterizer.is_wireframe, rasterizer.cull_mode, rasterizer.is_depth_bias);
}

VkPipelineMultisampleStateCreateInfo
GeneratePipelineMultisampleStateCreateInfo() {
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

VkPipelineColorBlendAttachmentState
GeneratePipelineColorBlendAttachmentState() {
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

VkPipelineColorBlendStateCreateInfo GeneratePipelineColorBlendStateCreateInfo(
    const VkPipelineColorBlendAttachmentState* color_blend_attachments,
    usize color_blend_attachment_count) {
  COMET_ASSERT(
      color_blend_attachment_count == 0 || color_blend_attachments != nullptr,
      "vulkan_initializer_utils::GeneratePipelineColorBlendStateCreateInfo",
      "color blend attachments are null");

  VkPipelineColorBlendStateCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  info.logicOpEnable = VK_FALSE;
  info.logicOp = VK_LOGIC_OP_COPY;
  info.attachmentCount = static_cast<u32>(color_blend_attachment_count);
  info.pAttachments = color_blend_attachments;
  info.blendConstants[0] = .0f;
  info.blendConstants[1] = .0f;
  info.blendConstants[2] = .0f;
  info.blendConstants[3] = .0f;
  info.pNext = VK_NULL_HANDLE;
  return info;
}

VkPipelineLayoutCreateInfo GeneratePipelineLayoutCreateInfo(
    const PipelineLayoutDescr& descr) {
  VkPipelineLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.flags = 0;

  if (descr.descriptor_set_layout_handles == nullptr ||
      descr.descriptor_set_layout_count == 0) {
    info.pSetLayouts = VK_NULL_HANDLE;
    info.setLayoutCount = 0;
  } else {
    info.pSetLayouts = descr.descriptor_set_layout_handles->GetData();
    info.setLayoutCount = static_cast<u32>(descr.descriptor_set_layout_count);
  }

  if (descr.push_constant_ranges == nullptr ||
      descr.push_constant_ranges->IsEmpty()) {
    info.pPushConstantRanges = VK_NULL_HANDLE;
    info.pushConstantRangeCount = 0;
  } else {
    info.pPushConstantRanges = descr.push_constant_ranges->GetData();
    info.pushConstantRangeCount =
        static_cast<u32>(descr.push_constant_ranges->GetSize());
  }

  return info;
}

VkPipelineDepthStencilStateCreateInfo
GeneratePipelineDepthStencilStateCreateInfo(bool is_depth_test,
                                            bool is_depth_write,
                                            VkCompareOp compare_op) {
  VkPipelineDepthStencilStateCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.depthTestEnable = is_depth_test;
  info.depthWriteEnable = is_depth_write;
  info.depthCompareOp = is_depth_test ? compare_op : VK_COMPARE_OP_ALWAYS;
  info.depthBoundsTestEnable = VK_FALSE;
  info.minDepthBounds = .0f;
  info.maxDepthBounds = 1.0f;
  info.stencilTestEnable = VK_FALSE;
  info.front = {};
  info.back = {};
  return info;
}

VkPipelineDepthStencilStateCreateInfo
GeneratePipelineDepthStencilStateCreateInfo(
    const DepthStencilState& depth_stencil) {
  return GeneratePipelineDepthStencilStateCreateInfo(
      depth_stencil.is_depth_test, depth_stencil.is_depth_write,
      GetVkCompareOp(depth_stencil.compare_op));
}

VkImageCreateInfo GenerateImageCreateInfo(
    u32 width, u32 height, u32 mip_levels, u32 array_layers,
    VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage_flags, VkSharingMode sharing_mode,
    const u32* queue_family_indices, u32 queue_family_index_count) {
  COMET_ASSERT(width > 0, "vulkan_initializer_utils::GenerateImageCreateInfo",
               "image width is zero");
  COMET_ASSERT(height > 0, "vulkan_initializer_utils::GenerateImageCreateInfo",
               "image height is zero");
  COMET_ASSERT(mip_levels > 0,
               "vulkan_initializer_utils::GenerateImageCreateInfo",
               "mip level count is zero");
  COMET_ASSERT(array_layers > 0,
               "vulkan_initializer_utils::GenerateImageCreateInfo",
               "array layer count is zero");
  COMET_ASSERT(format != VK_FORMAT_UNDEFINED,
               "vulkan_initializer_utils::GenerateImageCreateInfo",
               "image format is undefined");

  VkImageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.imageType = VK_IMAGE_TYPE_2D;
  info.extent.width = width;
  info.extent.height = height;
  info.extent.depth = 1;
  info.mipLevels = mip_levels;
  info.arrayLayers = array_layers;
  info.format = format;
  info.tiling = tiling;
  info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  info.usage = usage_flags;
  info.samples = num_samples;
  info.flags = 0;
  info.sharingMode = sharing_mode;
  info.queueFamilyIndexCount = queue_family_index_count;
  info.pQueueFamilyIndices = queue_family_indices;
  return info;
}

VkImageViewCreateInfo GenerateImageViewCreateInfo(
    VkImage image_handle, VkFormat format, VkImageAspectFlags aspect_flags,
    u32 mip_levels, u32 base_array_layer, u32 layer_count,
    VkImageViewType view_type) {
  COMET_ASSERT(image_handle != VK_NULL_HANDLE,
               "vulkan_initializer_utils::GenerateImageViewCreateInfo",
               "image handle is invalid");
  COMET_ASSERT(format != VK_FORMAT_UNDEFINED,
               "vulkan_initializer_utils::GenerateImageViewCreateInfo",
               "image format is undefined");
  COMET_ASSERT(mip_levels > 0,
               "vulkan_initializer_utils::GenerateImageViewCreateInfo",
               "mip level count is zero");
  COMET_ASSERT(layer_count > 0,
               "vulkan_initializer_utils::GenerateImageViewCreateInfo",
               "layer count is zero");

  VkImageViewCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  info.image = image_handle;
  info.viewType = view_type;
  info.format = format;

  info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  info.subresourceRange.aspectMask = aspect_flags;
  info.subresourceRange.baseMipLevel = 0;
  info.subresourceRange.levelCount = mip_levels;
  info.subresourceRange.baseArrayLayer = base_array_layer;
  info.subresourceRange.layerCount = layer_count;
  return info;
}

VkDescriptorSetLayoutCreateInfo GenerateDescriptorSetLayoutCreateInfo(
    const DescriptorSetLayoutBinding& descriptor_set_data) {
  COMET_ASSERT(
      descriptor_set_data.binding_count == 0 ||
          !descriptor_set_data.bindings.IsEmpty(),
      "vulkan_initializer_utils::GenerateDescriptorSetLayoutCreateInfo",
      "descriptor set layout bindings are empty");

  VkDescriptorSetLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  info.pBindings = descriptor_set_data.bindings.GetData();
  info.bindingCount = descriptor_set_data.binding_count;
  info.pNext = VK_NULL_HANDLE;
  return info;
}

VkDescriptorSetLayoutBinding GenerateDescriptorSetLayoutBinding(
    VkDescriptorType type, VkShaderStageFlags stage_flags, u32 binding) {
  VkDescriptorSetLayoutBinding layout_binding{};
  layout_binding.binding = binding;
  layout_binding.descriptorCount = 1;
  layout_binding.descriptorType = type;
  layout_binding.pImmutableSamplers = VK_NULL_HANDLE;
  layout_binding.stageFlags = stage_flags;
  return layout_binding;
}

VkDescriptorBufferInfo GenerateDescriptorBufferInfo(VkBuffer buffer_handle,
                                                    sptrdiff offset,
                                                    usize stride) {
  COMET_ASSERT(buffer_handle != VK_NULL_HANDLE,
               "vulkan_initializer_utils::GenerateDescriptorBufferInfo",
               "buffer handle is invalid");
  COMET_ASSERT(offset >= 0,
               "vulkan_initializer_utils::GenerateDescriptorBufferInfo",
               "buffer offset is invalid", "offset", offset);
  COMET_ASSERT(stride > 0,
               "vulkan_initializer_utils::GenerateDescriptorBufferInfo",
               "buffer range is zero");

  VkDescriptorBufferInfo info{};
  info.buffer = buffer_handle;
  info.offset = static_cast<VkDeviceSize>(offset);
  info.range = static_cast<VkDeviceSize>(stride);
  return info;
}

VkDescriptorImageInfo GenerateDescriptorImageInfo(VkSampler sampler_handle,
                                                  VkImageView image_view_handle,
                                                  VkImageLayout image_layout) {
  VkDescriptorImageInfo info{};
  info.sampler = sampler_handle;
  info.imageView = image_view_handle;
  info.imageLayout = image_layout;
  return info;
}

VkWriteDescriptorSet GenerateBufferWriteDescriptorSet(
    VkDescriptorType type, VkDescriptorSet dst_descriptor_set_handle,
    const VkDescriptorBufferInfo* buffer_info, u32 binding) {
  COMET_ASSERT(dst_descriptor_set_handle != VK_NULL_HANDLE,
               "vulkan_initializer_utils::GenerateBufferWriteDescriptorSet",
               "descriptor set handle is invalid");
  COMET_ASSERT(buffer_info != nullptr,
               "vulkan_initializer_utils::GenerateBufferWriteDescriptorSet",
               "buffer info is null");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = VK_NULL_HANDLE;
  write.dstBinding = binding;
  write.dstSet = dst_descriptor_set_handle;
  write.descriptorCount = 1;
  write.descriptorType = type;
  write.pBufferInfo = buffer_info;
  return write;
}

VkWriteDescriptorSet GenerateImageWriteDescriptorSet(
    VkDescriptorType type, VkDescriptorSet dst_descriptor_set_handle,
    const VkDescriptorImageInfo* image_info, u32 binding) {
  COMET_ASSERT(dst_descriptor_set_handle != VK_NULL_HANDLE,
               "vulkan_initializer_utils::GenerateImageWriteDescriptorSet",
               "descriptor set handle is invalid");
  COMET_ASSERT(image_info != nullptr,
               "vulkan_initializer_utils::GenerateImageWriteDescriptorSet",
               "image info is null");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = VK_NULL_HANDLE;
  write.dstBinding = binding;
  write.dstSet = dst_descriptor_set_handle;
  write.descriptorCount = 1;
  write.descriptorType = type;
  write.pImageInfo = image_info;
  return write;
}

VkWriteDescriptorSet GenerateImageWriteDescriptorSet(
    VkDescriptorType type, VkDescriptorSet dst_descriptor_set_handle,
    const VkDescriptorImageInfo* image_info_list, u32 image_info_count,
    u32 binding) {
  COMET_ASSERT(dst_descriptor_set_handle != VK_NULL_HANDLE,
               "vulkan_initializer_utils::GenerateImageWriteDescriptorSet",
               "descriptor set handle is invalid");
  COMET_ASSERT(image_info_count > 0,
               "vulkan_initializer_utils::GenerateImageWriteDescriptorSet",
               "image info count is zero");
  COMET_ASSERT(image_info_list != nullptr,
               "vulkan_initializer_utils::GenerateImageWriteDescriptorSet",
               "image info list is null");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = VK_NULL_HANDLE;
  write.dstBinding = binding;
  write.dstSet = dst_descriptor_set_handle;
  write.descriptorCount = image_info_count;
  write.descriptorType = type;
  write.pImageInfo = image_info_list;
  return write;
}

VkSamplerCreateInfo GenerateSamplerCreateInfo(
    VkFilter filters, VkSamplerAddressMode address_mode) {
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

VkSamplerCreateInfo GenerateBaseSamplerCreateInfo() {
  VkSamplerCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  info.pNext = VK_NULL_HANDLE;
  info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  info.unnormalizedCoordinates = VK_FALSE;
  info.compareEnable = VK_FALSE;
  info.compareOp = VK_COMPARE_OP_ALWAYS;
  info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  info.mipLodBias = .0f;
  info.minLod = .0f;
  info.maxLod = VK_LOD_CLAMP_NONE;
  return info;
}

VkSamplerCreateInfo GenerateSamplerCreateInfo(
    const resource::TextureMapResource& texture_map, bool is_sampler_anisotropy,
    f32 max_sampler_anisotropy) {
  auto info{GenerateBaseSamplerCreateInfo()};

  info.minFilter = GetFilter(texture_map.min_filter_mode);
  info.magFilter = GetFilter(texture_map.mag_filter_mode);
  info.addressModeU = GetSamplerAddressMode(texture_map.u_repeat_mode);
  info.addressModeV = GetSamplerAddressMode(texture_map.v_repeat_mode);
  info.addressModeW = GetSamplerAddressMode(texture_map.w_repeat_mode);
  info.anisotropyEnable = is_sampler_anisotropy ? VK_TRUE : VK_FALSE;
  info.maxAnisotropy = max_sampler_anisotropy;

  return info;
}

VkBufferMemoryBarrier GenerateBufferMemoryBarrier(
    const Buffer& buffer, VkAccessFlags src_access_mask,
    VkAccessFlags dst_access_mask, u32 src_queue_family_index,
    u32 dst_queue_family_index, VkDeviceSize offset, VkDeviceSize size) {
  return GenerateBufferMemoryBarrier(buffer.handle, src_access_mask,
                                     dst_access_mask, src_queue_family_index,
                                     dst_queue_family_index, offset, size);
}

VkBufferMemoryBarrier GenerateBufferMemoryBarrier(
    VkBuffer buffer_handle, VkAccessFlags src_access_mask,
    VkAccessFlags dst_access_mask, u32 src_queue_family_index,
    u32 dst_queue_family_index, VkDeviceSize offset, VkDeviceSize size) {
  VkBufferMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  barrier.pNext = VK_NULL_HANDLE;
  barrier.srcAccessMask = src_access_mask;
  barrier.dstAccessMask = dst_access_mask;
  barrier.srcQueueFamilyIndex = src_queue_family_index;
  barrier.dstQueueFamilyIndex = dst_queue_family_index;
  barrier.buffer = buffer_handle;
  barrier.offset = offset;
  barrier.size = size;
  return barrier;
}

VkTimelineSemaphoreSubmitInfo GenerateTimelineSemaphoreSubmitInfo(
    u32 wait_semaphore_value_count, const u64* wait_semaphore_values,
    u32 signal_semaphore_value_count, const u64* signal_semaphore_values) {
  COMET_ASSERT(
      wait_semaphore_value_count == 0 || wait_semaphore_values != nullptr,
      "vulkan_initializer_utils::GenerateTimelineSemaphoreSubmitInfo",
      "wait semaphore values are null");
  COMET_ASSERT(
      signal_semaphore_value_count == 0 || signal_semaphore_values != nullptr,
      "vulkan_initializer_utils::GenerateTimelineSemaphoreSubmitInfo",
      "signal semaphore values are null");

  VkTimelineSemaphoreSubmitInfo info{};
  info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
  info.waitSemaphoreValueCount = wait_semaphore_value_count;
  info.pWaitSemaphoreValues = wait_semaphore_values;
  info.signalSemaphoreValueCount = signal_semaphore_value_count;
  info.pSignalSemaphoreValues = signal_semaphore_values;
  return info;
}
}  // namespace init
}  // namespace vk
}  // namespace rendering
}  // namespace comet