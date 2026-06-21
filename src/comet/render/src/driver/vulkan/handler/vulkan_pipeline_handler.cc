// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/handler/vulkan_pipeline_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/allocator/allocator.h"
#include "comet/render/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/render/driver/vulkan/vulkan_context.h"
#include "comet/render/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace render {
namespace vk {

PipelineHandler::PipelineHandler(const PipelineHandlerDescr& descr)
    : Handler{descr}, render_pass_handler_{descr.render_pass_handler} {
  COMET_ASSERT(render_pass_handler_ != nullptr,
               "PipelineHandler::PipelineHandler",
               "render pass handler is null");
}

PipelineLayoutHandle PipelineHandler::GenerateLayout(
    const PipelineLayoutDescr& descr) {
  const auto handle{layout_pool_.Generate()};
  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= layouts_.GetSize()) {
    layouts_.Resize(index + 1);
  }

  auto* layout{allocator_.AllocateOneAndPopulate<PipelineLayout>()};
  layout->handle = PipelineLayoutHandle::Invalid();

  const auto info{init::GeneratePipelineLayoutCreateInfo(descr)};

  COMET_CHECK_VK(vkCreatePipelineLayout(context_->GetDevice(), &info, nullptr,
                                        &layout->native_handle),
                 "PipelineHandler::GenerateLayout",
                 "pipeline layout creation failed");

  layouts_[index] = layout;
  layout->handle = handle;
  return handle;
}

PipelineHandle PipelineHandler::Generate(const GraphicsPipelineDescr& descr) {
  COMET_ASSERT(descr.layout_handle, "PipelineHandler::Generate",
               "pipeline layout handle is invalid");

  const auto handle{pipeline_pool_.Generate()};
  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= pipelines_.GetSize()) {
    pipelines_.Resize(index + 1);
  }

  auto* pipeline{allocator_.AllocateOneAndPopulate<Pipeline>()};
  pipeline->handle = PipelineHandle::Invalid();
  pipeline->type = PipelineBindType::Graphics;

  pipeline->layout_handle = descr.layout_handle;
  const auto* layout{GetLayout(pipeline->layout_handle)};

  auto& device{context_->GetDevice()};

  VkPipelineViewportStateCreateInfo viewport_info{};
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = &descr.viewport;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = &descr.scissor;

  const auto color_blend_info{init::GeneratePipelineColorBlendStateCreateInfo(
      &descr.color_blend_attachment_state, 1)};

  constexpr StaticArray<VkDynamicState, 3> kDynamicStates{
      VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_DEPTH_BIAS};

  VkPipelineDynamicStateCreateInfo dynamic_state_info{};
  dynamic_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_info.dynamicStateCount =
      static_cast<u32>(kDynamicStates.GetSize());
  dynamic_state_info.pDynamicStates = kDynamicStates.GetData();

  VkPipelineVertexInputStateCreateInfo vertex_input_state_info{};

  if (descr.vertex_attributes != nullptr &&
      !descr.vertex_attributes->IsEmpty()) {
    vertex_input_state_info = init::GeneratePipelineVertexInputStateCreateInfo(
        &descr.vertex_input_binding_description, 1,
        descr.vertex_attributes->GetData(),
        static_cast<u32>(descr.vertex_attributes->GetSize()));
  } else {
    vertex_input_state_info = init::GeneratePipelineVertexInputStateCreateInfo(
        nullptr, 0, nullptr, 0);
  }

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = static_cast<u32>(descr.shader_stages.GetSize());
  pipeline_info.pStages = descr.shader_stages.GetData();
  pipeline_info.pVertexInputState = &vertex_input_state_info;
  pipeline_info.pInputAssemblyState = &descr.input_assembly_state;
  pipeline_info.pViewportState = &viewport_info;
  pipeline_info.pRasterizationState = &descr.rasterization_state;
  pipeline_info.pMultisampleState = &descr.multisample_state;
  pipeline_info.pDepthStencilState = &descr.depth_stencil_state;
  pipeline_info.pColorBlendState = &color_blend_info;
  pipeline_info.pDynamicState = &dynamic_state_info;
  pipeline_info.layout = layout->native_handle;
  pipeline_info.renderPass =
      render_pass_handler_->GetVkHandle(descr.render_pass_handle);
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = -1;

  COMET_CHECK_VK(
      vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info,
                                nullptr, &pipeline->native_handle),
      "PipelineHandler::Generate", "graphics pipeline creation failed");

  pipelines_[index] = pipeline;
  pipeline->handle = handle;
  return handle;
}

PipelineHandle PipelineHandler::Generate(const ComputePipelineDescr& descr) {
  COMET_ASSERT(descr.layout_handle, "PipelineHandler::Generate",
               "pipeline layout handle is invalid");

  const auto handle{pipeline_pool_.Generate()};
  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= pipelines_.GetSize()) {
    pipelines_.Resize(index + 1);
  }

  auto* pipeline{allocator_.AllocateOneAndPopulate<Pipeline>()};
  pipeline->handle = PipelineHandle::Invalid();
  pipeline->type = PipelineBindType::Compute;

  pipeline->layout_handle = descr.layout_handle;
  const auto* layout{GetLayout(pipeline->layout_handle)};

  VkComputePipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipeline_info.stage = descr.shader_stage;
  pipeline_info.layout = layout->native_handle;
  pipeline_info.flags = 0;
  pipeline_info.pNext = nullptr;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = -1;

  COMET_CHECK_VK(vkCreateComputePipelines(context_->GetDevice(), VK_NULL_HANDLE,
                                          1, &pipeline_info, nullptr,
                                          &pipeline->native_handle),
                 "PipelineHandler::Generate",
                 "compute pipeline creation failed");

  pipelines_[index] = pipeline;
  pipeline->handle = handle;
  return handle;
}

void PipelineHandler::DestroyLayout(PipelineLayoutHandle handle) {
  auto* layout{TryGetLayout(handle)};

  if (layout == nullptr) {
    return;
  }

  COMET_ASSERT(layout->handle == handle, "PipelineHandler::DestroyLayout",
               "pipeline layout handle mismatch", "expected_handle", handle,
               "actual_handle", layout->handle);

  layouts_[handle.GetIndex()] = nullptr;

  layout_pool_.Destroy(handle);
  COMET_ASSERT(!layout_pool_.IsAlive(handle), "PipelineHandler::DestroyLayout",
               "pipeline layout handle is still alive", "handle", handle);

  DestroyPipelineLayoutObject(layout);
}

void PipelineHandler::Destroy(PipelineHandle handle) {
  auto* pipeline{TryGet(handle)};

  if (pipeline == nullptr) {
    return;
  }

  COMET_ASSERT(pipeline->handle == handle, "PipelineHandler::Destroy",
               "pipeline handle mismatch", "expected_handle", handle,
               "actual_handle", pipeline->handle);

  if (bound_pipeline_ == handle) {
    bound_pipeline_.Invalidate();
  }

  pipelines_[handle.GetIndex()] = nullptr;

  pipeline_pool_.Destroy(handle);
  COMET_ASSERT(!pipeline_pool_.IsAlive(handle), "PipelineHandler::Destroy",
               "pipeline handle is still alive", "handle", handle);

  DestroyPipelineObject(pipeline);
}

void PipelineHandler::Bind(PipelineHandle handle) {
  if (handle == bound_pipeline_) {
    return;
  }

  const auto* pipeline{Get(handle)};
  COMET_ASSERT(pipeline->native_handle != VK_NULL_HANDLE,
               "PipelineHandler::Bind", "pipeline native handle is invalid",
               "pipeline_handle", handle);

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};

  vkCmdBindPipeline(frame_data.command_buffer_handle,
                    pipeline->type == PipelineBindType::Graphics
                        ? VK_PIPELINE_BIND_POINT_GRAPHICS
                        : VK_PIPELINE_BIND_POINT_COMPUTE,
                    pipeline->native_handle);

  bound_pipeline_ = handle;
}

void PipelineHandler::Reset() { bound_pipeline_.Invalidate(); }

VkPipelineLayout PipelineHandler::GetNativeLayoutHandle(
    PipelineHandle handle) const {
  const auto* pipeline{Get(handle)};
  return GetLayout(pipeline->layout_handle)->native_handle;
}

VkPipelineLayout PipelineHandler::GetNativeLayoutHandle(
    PipelineLayoutHandle handle) const {
  return GetLayout(handle)->native_handle;
}

VkPipeline PipelineHandler::GetNativeHandle(PipelineHandle handle) const {
  return Get(handle)->native_handle;
}

PipelineBindType PipelineHandler::GetBindType(PipelineHandle handle) const {
  return Get(handle)->type;
}

void PipelineHandler::OnInitialize() {
  allocator_.Initialize();

  constexpr usize kDefaultCapacity(16);
  pipelines_ = Array<Pipeline*>::WithCapacity(&allocator_, kDefaultCapacity);
  layouts_ =
      Array<PipelineLayout*>::WithCapacity(&allocator_, kDefaultCapacity);

  bound_pipeline_.Invalidate();
}

void PipelineHandler::OnShutdown() {
  for (auto* pipeline : pipelines_) {
    if (pipeline != nullptr) {
      DestroyPipelineObject(pipeline);
    }
  }

  for (auto* layout : layouts_) {
    if (layout != nullptr) {
      DestroyPipelineLayoutObject(layout);
    }
  }

  pipelines_.Release();
  layouts_.Release();

  pipeline_pool_.Destroy();
  layout_pool_.Destroy();

  bound_pipeline_.Invalidate();

  allocator_.Destroy();
}

Pipeline* PipelineHandler::Get(PipelineHandle handle) {
  auto* pipeline{TryGet(handle)};
  COMET_ASSERT(pipeline != nullptr, "PipelineHandler::Get",
               "pipeline not found", "pipeline_handle", handle);
  return pipeline;
}

const Pipeline* PipelineHandler::Get(PipelineHandle handle) const {
  const auto* pipeline{TryGet(handle)};
  COMET_ASSERT(pipeline != nullptr, "PipelineHandler::Get",
               "pipeline not found", "pipeline_handle", handle);
  return pipeline;
}

Pipeline* PipelineHandler::TryGet(PipelineHandle handle) {
  if (!pipeline_pool_.IsAlive(handle)) {
    return nullptr;
  }

  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= pipelines_.GetSize()) {
    return nullptr;
  }

  return pipelines_[index];
}

const Pipeline* PipelineHandler::TryGet(PipelineHandle handle) const {
  if (!pipeline_pool_.IsAlive(handle)) {
    return nullptr;
  }

  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= pipelines_.GetSize()) {
    return nullptr;
  }

  return pipelines_[index];
}

PipelineLayout* PipelineHandler::GetLayout(PipelineLayoutHandle handle) {
  auto* layout{TryGetLayout(handle)};
  COMET_ASSERT(layout != nullptr, "PipelineHandler::GetLayout",
               "pipeline layout not found", "pipeline_layout_handle", handle);
  return layout;
}

const PipelineLayout* PipelineHandler::GetLayout(
    PipelineLayoutHandle handle) const {
  const auto* layout{TryGetLayout(handle)};
  COMET_ASSERT(layout != nullptr, "PipelineHandler::GetLayout",
               "pipeline layout not found", "pipeline_layout_handle", handle);
  return layout;
}

PipelineLayout* PipelineHandler::TryGetLayout(PipelineLayoutHandle handle) {
  if (!layout_pool_.IsAlive(handle)) {
    return nullptr;
  }

  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= layouts_.GetSize()) {
    return nullptr;
  }

  return layouts_[index];
}

const PipelineLayout* PipelineHandler::TryGetLayout(
    PipelineLayoutHandle handle) const {
  if (!layout_pool_.IsAlive(handle)) {
    return nullptr;
  }

  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= layouts_.GetSize()) {
    return nullptr;
  }

  return layouts_[index];
}

void PipelineHandler::DestroyPipelineObject(Pipeline* pipeline) {
  if (pipeline == nullptr) {
    return;
  }

  if (pipeline->native_handle != VK_NULL_HANDLE) {
    vkDestroyPipeline(context_->GetDevice(), pipeline->native_handle, nullptr);
    pipeline->native_handle = VK_NULL_HANDLE;
  }

  pipeline->layout_handle.Invalidate();
  pipeline->type = PipelineBindType::Unknown;
  pipeline->handle.Invalidate();
  allocator_.Deallocate(pipeline);
}

void PipelineHandler::DestroyPipelineLayoutObject(PipelineLayout* layout) {
  if (layout == nullptr) {
    return;
  }

  if (layout->native_handle != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(context_->GetDevice(), layout->native_handle,
                            nullptr);
    layout->native_handle = VK_NULL_HANDLE;
  }

  layout->handle.Invalidate();
  allocator_.Deallocate(layout);
}
}  // namespace vk
}  // namespace render
}  // namespace comet