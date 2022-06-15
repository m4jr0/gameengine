// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_PIPELINE_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_PIPELINE_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/vulkan_mesh.h"

namespace comet {
namespace rendering {
namespace vk {
enum class VulkanPipelineType { Unknown = 0, Graphics, Compute };

struct VulkanPipelineDescr {
  VulkanPipelineType pipeline_type{VulkanPipelineType::Unknown};
  VkDevice device{VK_NULL_HANDLE};
  VkRenderPass render_pass{VK_NULL_HANDLE};

  VkPipelineLayout layout;
  VkViewport viewport;
  VkRect2D scissor;
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
  VertexInputDescription vertex_input_description;

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
  VkPipelineRasterizationStateCreateInfo rasterization_state;
  VkPipelineColorBlendAttachmentState color_blend_attachment_state;
  VkPipelineMultisampleStateCreateInfo multisample_state;
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
};

VkPipeline GeneratePipeline(const VulkanPipelineDescr& descr);
VkPipeline GenerateGraphicsPipeline(const VulkanPipelineDescr& descr);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_PIPELINE_H_
