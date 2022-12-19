// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_PIPELINE_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_PIPELINE_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/data/vulkan_mesh.h"
#include "comet/rendering/driver/vulkan/data/vulkan_render_pass.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader_data.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
enum class PipelineType { Unknown = 0, Graphics, Compute };
using PipelineId = stringid::StringId;
constexpr auto kInvalidPipelineId{static_cast<PipelineId>(-1)};

struct PipelineDescr {
  u8 descriptor_set_layout_count{0};
  PipelineId id{kInvalidPipelineId};
  PipelineType type{PipelineType::Unknown};
  const RenderPass* render_pass{nullptr};

  VkViewport viewport;
  VkRect2D scissor;
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
  VkVertexInputBindingDescription vertex_input_binding_description;
  std::vector<VkVertexInputAttributeDescription>* vertex_attributes{nullptr};
  std::array<VkDescriptorSetLayout, kDescriptorSetMaxLayoutCount>*
      descriptor_set_layout_handles{nullptr};
  std::vector<VkPushConstantRange>* push_constant_ranges{nullptr};

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
  VkPipelineRasterizationStateCreateInfo rasterization_state;
  VkPipelineColorBlendAttachmentState color_blend_attachment_state;
  VkPipelineMultisampleStateCreateInfo multisample_state;
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
};

struct Pipeline {
  PipelineId id{kInvalidPipelineId};
  PipelineType type{PipelineType::Unknown};
  VkPipelineLayout layout_handle{VK_NULL_HANDLE};
  VkPipeline handle{VK_NULL_HANDLE};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet
#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_PIPELINE_H_
