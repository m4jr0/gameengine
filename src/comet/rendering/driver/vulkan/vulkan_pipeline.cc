// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_pipeline.h"

#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/driver/vulkan/vulkan_initializers.h"

namespace comet {
namespace rendering {
namespace vk {
VkPipeline GenerateGraphicsPipeline(const VulkanPipelineDescr& descr) {
  COMET_ASSERT(descr.pipeline_type == VulkanPipelineType::Graphics,
               "Type provided for graphics pipeline generation is wrong: ",
               static_cast<std::underlying_type_t<VulkanPipelineType>>(
                   descr.pipeline_type),
               "!");

  VkPipelineViewportStateCreateInfo viewport_info{};
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = &descr.viewport;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = &descr.scissor;

  const auto color_blend_info{init::GetPipelineColorBlendStateCreateInfo(
      &descr.color_blend_attachment_state, 1)};

  std::array<VkDynamicState, 3> dynamic_states{{VK_DYNAMIC_STATE_VIEWPORT,
                                                VK_DYNAMIC_STATE_SCISSOR,
                                                VK_DYNAMIC_STATE_DEPTH_BIAS}};

  VkPipelineDynamicStateCreateInfo dynamic_state_info{};
  dynamic_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_info.dynamicStateCount = dynamic_states.size();
  dynamic_state_info.pDynamicStates = dynamic_states.data();

  auto vertex_input_state_info{init::GetPipelineVertexInputStateCreateInfo(
      descr.vertex_input_description.bindings.data(),
      descr.vertex_input_description.bindings.size(),
      descr.vertex_input_description.attributes.data(),
      descr.vertex_input_description.attributes.size())};

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = static_cast<u32>(descr.shader_stages.size());
  pipeline_info.pStages = descr.shader_stages.data();
  pipeline_info.pNext = VK_NULL_HANDLE;

  pipeline_info.pVertexInputState = &vertex_input_state_info;
  pipeline_info.pInputAssemblyState = &descr.input_assembly_state;
  pipeline_info.pViewportState = &viewport_info;
  pipeline_info.pRasterizationState = &descr.rasterization_state;
  pipeline_info.pMultisampleState = &descr.multisample_state;
  pipeline_info.pDepthStencilState = &descr.depth_stencil_state;
  pipeline_info.pColorBlendState = &color_blend_info;
  pipeline_info.pDynamicState = &dynamic_state_info;

  pipeline_info.layout = descr.layout;
  pipeline_info.renderPass = descr.render_pass;
  pipeline_info.subpass = 0;

  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = -1;

  VkPipeline pipeline{VK_NULL_HANDLE};

  COMET_CHECK_VK(
      vkCreateGraphicsPipelines(descr.device, VK_NULL_HANDLE, 1, &pipeline_info,
                                VK_NULL_HANDLE, &pipeline),
      "Failed to create graphics pipeline!");

  return pipeline;
}

VkPipeline GeneratePipeline(const VulkanPipelineDescr& descr) {
  switch (descr.pipeline_type) {
    case VulkanPipelineType::Graphics:
      return GenerateGraphicsPipeline(descr);
  }

  COMET_ASSERT(false, "Invalid Vulkan pipeline type provided: ",
               static_cast<std::underlying_type_t<VulkanPipelineType>>(
                   descr.pipeline_type),
               "!");

  return VK_NULL_HANDLE;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet