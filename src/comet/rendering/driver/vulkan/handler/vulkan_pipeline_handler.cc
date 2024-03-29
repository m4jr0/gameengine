// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_pipeline_handler.h"

#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
PipelineHandler::PipelineHandler(const PipelineHandlerDescr& descr)
    : Handler{descr} {}

void PipelineHandler::Shutdown() {
  for (auto& it : pipelines_) {
    Destroy(it.second, true);
  }

  pipelines_.clear();
  Handler::Shutdown();
}

const Pipeline* PipelineHandler::Generate(const PipelineDescr& descr) {
  switch (descr.type) {
    case PipelineType::Graphics:
      return GenerateGraphics(descr);
    default:
      COMET_ASSERT(
          false, "Invalid  pipeline type provided: ",
          static_cast<std::underlying_type_t<PipelineType>>(descr.type), "!");

      return nullptr;
  }
}

const Pipeline* PipelineHandler::Get(PipelineId pipeline_id) const {
  const auto* pipeline{TryGet(pipeline_id)};
  COMET_ASSERT(pipeline != nullptr,
               "Requested pipeline does not exist: ", pipeline_id, "!");
  return pipeline;
}

const Pipeline* PipelineHandler::TryGet(PipelineId pipeline_id) const {
  auto it{pipelines_.find(pipeline_id)};

  if (it == pipelines_.end()) {
    return nullptr;
  }

  return &it->second;
}

const Pipeline* PipelineHandler::GetOrGenerate(const PipelineDescr& descr) {
  auto* pipeline{TryGet(descr.id)};

  if (pipeline != nullptr) {
    return pipeline;
  }

  return Generate(descr);
}

void PipelineHandler::Destroy(PipelineId pipeline_id) {
  Destroy(*Get(pipeline_id));
}

void PipelineHandler::Destroy(Pipeline& pipeline) {
  return Destroy(pipeline, false);
}

void PipelineHandler::Bind(const Pipeline& pipeline) const {
  COMET_ASSERT(pipeline.handle != VK_NULL_HANDLE,
               "Pipeline handle is null! Unable to bind!");

  vkCmdBindPipeline(context_->GetFrameData().command_buffer_handle,
                    VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
}

const Pipeline* PipelineHandler::GenerateGraphics(const PipelineDescr& descr) {
  COMET_ASSERT(descr.type == PipelineType::Graphics,
               "Type provided for graphics pipeline generation is wrong: ",
               static_cast<std::underlying_type_t<PipelineType>>(descr.type),
               "!");

  Pipeline pipeline{};
  pipeline.id = descr.id;
  pipeline.type = descr.type;

  auto& device{context_->GetDevice()};

  auto pipeline_layout_create_info{
      init::GeneratePipelineLayoutCreateInfo(descr)};
  COMET_CHECK_VK(
      vkCreatePipelineLayout(device, &pipeline_layout_create_info,
                             VK_NULL_HANDLE, &pipeline.layout_handle),
      "Unable to create pipeline layout!");

  VkPipelineViewportStateCreateInfo viewport_info{};
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = &descr.viewport;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = &descr.scissor;

  const auto color_blend_info{init::GeneratePipelineColorBlendStateCreateInfo(
      &descr.color_blend_attachment_state, 1)};

  constexpr std::array<VkDynamicState, 3> dynamic_states{
      {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
       VK_DYNAMIC_STATE_DEPTH_BIAS}};

  VkPipelineDynamicStateCreateInfo dynamic_state_info{};
  dynamic_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_info.dynamicStateCount =
      static_cast<u32>(dynamic_states.size());
  dynamic_state_info.pDynamicStates = dynamic_states.data();

  auto vertex_input_state_info{init::GeneratePipelineVertexInputStateCreateInfo(
      &descr.vertex_input_binding_description, 1,
      descr.vertex_attributes->data(),
      static_cast<u32>(descr.vertex_attributes->size()))};

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

  pipeline_info.layout = pipeline.layout_handle;
  pipeline_info.renderPass = descr.render_pass->handle;
  pipeline_info.subpass = 0;

  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = -1;

  COMET_CHECK_VK(
      vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info,
                                VK_NULL_HANDLE, &pipeline.handle),
      "Failed to create graphics pipeline!");

  auto insert_pair{pipelines_.emplace(pipeline.id, pipeline)};
  COMET_ASSERT(insert_pair.second, "Could not insert pipeline: ",
               COMET_STRING_ID_LABEL(pipeline.id), "!");
  return &insert_pair.first->second;
}

Pipeline* PipelineHandler::Get(PipelineId pipeline_id) {
  auto* pipeline{TryGet(pipeline_id)};
  COMET_ASSERT(pipeline != nullptr,
               "Requested pipeline does not exist: ", pipeline_id, "!");
  return pipeline;
}

Pipeline* PipelineHandler::TryGet(PipelineId pipeline_id) {
  auto it{pipelines_.find(pipeline_id)};

  if (it == pipelines_.end()) {
    return nullptr;
  }

  return &it->second;
}

void PipelineHandler::Destroy(Pipeline& pipeline, bool is_destroying_handler) {
  if (pipeline.handle != VK_NULL_HANDLE) {
    vkDestroyPipeline(context_->GetDevice(), pipeline.handle, VK_NULL_HANDLE);
    pipeline.handle = VK_NULL_HANDLE;
  }

  if (pipeline.layout_handle != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(context_->GetDevice(), pipeline.layout_handle,
                            VK_NULL_HANDLE);
    pipeline.layout_handle = VK_NULL_HANDLE;
  }

  if (!is_destroying_handler) {
    pipelines_.erase(pipeline.id);
  }

  pipeline.id = kInvalidPipelineId;
  pipeline.type = PipelineType::Unknown;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet