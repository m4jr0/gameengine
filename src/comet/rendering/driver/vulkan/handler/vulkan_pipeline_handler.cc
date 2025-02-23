// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_pipeline_handler.h"

#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
PipelineHandler::PipelineHandler(const PipelineHandlerDescr& descr)
    : Handler{descr} {}

void PipelineHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();
  pipelines_ = Map<PipelineId, Pipeline*>{&allocator_};
  pipeline_layouts_ = Map<PipelineLayoutId, PipelineLayout*>{&allocator_};
}

void PipelineHandler::Shutdown() {
  for (auto& it : pipelines_) {
    Destroy(it.value, true);
  }

  for (auto& it : pipeline_layouts_) {
    DestroyLayout(it.value, true);
  }

  pipelines_.Clear();
  pipeline_layouts_.Clear();
  allocator_.Destroy();
  bound_pipeline_ = nullptr;
  Handler::Shutdown();
}

const PipelineLayout* PipelineHandler::GenerateLayout(
    const PipelineLayoutDescr& descr) {
  auto* pipeline_layout{allocator_.AllocateOneAndPopulate<PipelineLayout>()};
  pipeline_layout->id = pipeline_layout_id_counter_++;

  auto pipeline_layout_create_info{
      init::GeneratePipelineLayoutCreateInfo(descr)};

  COMET_CHECK_VK(vkCreatePipelineLayout(
                     context_->GetDevice(), &pipeline_layout_create_info,
                     VK_NULL_HANDLE, &pipeline_layout->handle),
                 "Unable to create graphics pipeline layout!");

  return pipeline_layouts_.Emplace(pipeline_layout->id, pipeline_layout).value;
}

const Pipeline* PipelineHandler::Generate(const GraphicsPipelineDescr& descr) {
  auto* pipeline{allocator_.AllocateOneAndPopulate<Pipeline>()};
  pipeline->id = pipeline_id_counter_++;
  pipeline->type = PipelineType::Graphics;
  pipeline->layout_handle = descr.layout_handle;
  auto& device{context_->GetDevice()};

  VkPipelineViewportStateCreateInfo viewport_info{};
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = &descr.viewport;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = &descr.scissor;

  const auto color_blend_info{init::GeneratePipelineColorBlendStateCreateInfo(
      &descr.color_blend_attachment_state, 1)};

  constexpr StaticArray<VkDynamicState, 3> dynamic_states{
      VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_DEPTH_BIAS};

  VkPipelineDynamicStateCreateInfo dynamic_state_info{};
  dynamic_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_info.dynamicStateCount =
      static_cast<u32>(dynamic_states.GetSize());
  dynamic_state_info.pDynamicStates = dynamic_states.GetData();

  auto vertex_input_state_info{init::GeneratePipelineVertexInputStateCreateInfo(
      &descr.vertex_input_binding_description, 1,
      descr.vertex_attributes->GetData(),
      static_cast<u32>(descr.vertex_attributes->GetSize()))};

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = static_cast<u32>(descr.shader_stages.GetSize());
  pipeline_info.pStages = descr.shader_stages.GetData();
  pipeline_info.pNext = VK_NULL_HANDLE;

  pipeline_info.pVertexInputState = &vertex_input_state_info;
  pipeline_info.pInputAssemblyState = &descr.input_assembly_state;
  pipeline_info.pViewportState = &viewport_info;
  pipeline_info.pRasterizationState = &descr.rasterization_state;
  pipeline_info.pMultisampleState = &descr.multisample_state;
  pipeline_info.pDepthStencilState = &descr.depth_stencil_state;
  pipeline_info.pColorBlendState = &color_blend_info;
  pipeline_info.pDynamicState = &dynamic_state_info;

  pipeline_info.layout = pipeline->layout_handle;
  pipeline_info.renderPass = descr.render_pass->handle;
  pipeline_info.subpass = 0;

  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = -1;

  COMET_CHECK_VK(
      vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info,
                                VK_NULL_HANDLE, &pipeline->handle),
      "Failed to create graphics pipeline!");

  return pipelines_.Emplace(pipeline->id, pipeline).value;
}

const Pipeline* PipelineHandler::Generate(const ComputePipelineDescr& descr) {
  auto* pipeline{allocator_.AllocateOneAndPopulate<Pipeline>()};
  pipeline->id = pipeline_id_counter_++;
  pipeline->type = PipelineType::Compute;
  pipeline->layout_handle = descr.layout_handle;
  auto& device{context_->GetDevice()};

  VkComputePipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipeline_info.stage = descr.shader_stage;
  pipeline_info.layout = pipeline->layout_handle;
  pipeline_info.flags = 0;
  pipeline_info.pNext = VK_NULL_HANDLE;

  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = -1;

  COMET_CHECK_VK(
      vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline_info,
                               VK_NULL_HANDLE, &pipeline->handle),
      "Failed to create graphics pipeline!");

  return pipelines_.Emplace(pipeline->id, pipeline).value;
}

void PipelineHandler::DestroyLayout(PipelineLayoutId pipeline_layout_id) {
  DestroyLayout(GetLayout(pipeline_layout_id));
}

void PipelineHandler::DestroyLayout(PipelineLayout* pipeline_layout) {
  DestroyLayout(pipeline_layout, false);
}

void PipelineHandler::Destroy(PipelineId pipeline_id) {
  Destroy(Get(pipeline_id));
}

void PipelineHandler::Destroy(Pipeline* pipeline) { Destroy(pipeline, false); }

void PipelineHandler::Bind(const Pipeline* pipeline) {
  if (pipeline == bound_pipeline_) {
    return;
  }

  COMET_ASSERT(pipeline->handle != VK_NULL_HANDLE,
               "Pipeline handle is null! Unable to bind!");

  vkCmdBindPipeline(context_->GetFrameData().command_buffer_handle,
                    pipeline->type == PipelineType::Graphics
                        ? VK_PIPELINE_BIND_POINT_GRAPHICS
                        : VK_PIPELINE_BIND_POINT_COMPUTE,
                    pipeline->handle);

  bound_pipeline_ = pipeline;
}

void PipelineHandler::Reset() { bound_pipeline_ = nullptr; }

Pipeline* PipelineHandler::Get(PipelineId pipeline_id) {
  auto* pipeline{TryGet(pipeline_id)};
  COMET_ASSERT(pipeline != nullptr,
               "Requested pipeline does not exist: ", pipeline_id, "!");
  return pipeline;
}

Pipeline* PipelineHandler::TryGet(PipelineId pipeline_id) {
  auto** pipeline{pipelines_.TryGet(pipeline_id)};

  if (pipeline == nullptr) {
    return nullptr;
  }

  return *pipeline;
}

PipelineLayout* PipelineHandler::GetLayout(
    PipelineLayoutId pipeline_layout_id) {
  auto* pipeline_layout{TryGetLayout(pipeline_layout_id)};
  COMET_ASSERT(pipeline_layout != nullptr,
               "Requested pipeline layout does not exist: ", pipeline_layout_id,
               "!");
  return pipeline_layout;
}

PipelineLayout* PipelineHandler::TryGetLayout(
    PipelineLayoutId pipeline_layout_id) {
  auto** pipeline_layout{pipeline_layouts_.TryGet(pipeline_layout_id)};

  if (pipeline_layout == nullptr) {
    return nullptr;
  }

  return *pipeline_layout;
}

void PipelineHandler::Destroy(Pipeline* pipeline, bool is_destroying_handler) {
  if (pipeline->handle != VK_NULL_HANDLE) {
    vkDestroyPipeline(context_->GetDevice(), pipeline->handle, VK_NULL_HANDLE);
  }

  if (!is_destroying_handler) {
    pipelines_.Remove(pipeline->id);
  }

  allocator_.Deallocate(pipeline);
}

void PipelineHandler::DestroyLayout(PipelineLayout* pipeline_layout,
                                    bool is_destroying_handler) {
  if (pipeline_layout->handle != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(context_->GetDevice(), pipeline_layout->handle,
                            VK_NULL_HANDLE);
  }

  if (!is_destroying_handler) {
    pipeline_layouts_.Remove(pipeline_layout->id);
  }

  allocator_.Deallocate(pipeline_layout);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet