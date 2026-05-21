// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_PIPELINE_H_
#define COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_PIPELINE_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/type/vulkan_descriptor.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/rendering/type/pipeline.h"

namespace comet {
namespace rendering {
namespace vk {
struct RasterizerState {
  bool is_wireframe{false};
  bool is_depth_bias{false};
  CullMode cull_mode{CullMode::Unknown};
};

struct DepthStencilState {
  bool is_depth_test{true};
  bool is_depth_write{true};
  CompareOp compare_op{CompareOp::Less};
};

struct PipelineLayout {
  VkPipelineLayout native_handle{VK_NULL_HANDLE};
  PipelineLayoutHandle handle{};
};

struct PipelineLayoutDescr {
  u32 descriptor_set_layout_count{0};
  StaticArray<VkDescriptorSetLayout, kDescriptorSetMaxLayoutCount>*
      descriptor_set_layout_handles{nullptr};
  Array<VkPushConstantRange>* push_constant_ranges{nullptr};
};

struct ComputePipelineDescr {
  VkPipelineShaderStageCreateInfo shader_stage{};
  PipelineLayoutHandle layout_handle{};
};

struct GraphicsPipelineDescr {
  RenderPassHandle render_pass_handle{};

  VkViewport viewport{};
  VkRect2D scissor{};
  Array<VkPipelineShaderStageCreateInfo> shader_stages{};
  VkVertexInputBindingDescription vertex_input_binding_description{};
  Array<VkVertexInputAttributeDescription>* vertex_attributes{nullptr};

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state{};
  VkPipelineRasterizationStateCreateInfo rasterization_state{};
  VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
  VkPipelineMultisampleStateCreateInfo multisample_state{};
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state{};

  PipelineLayoutHandle layout_handle{};
};

struct Pipeline {
  PipelineBindType type{PipelineBindType::Unknown};
  VkPipeline native_handle{VK_NULL_HANDLE};
  PipelineLayoutHandle layout_handle{};
  PipelineHandle handle{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet
#endif  // COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_PIPELINE_H_
