// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDER_VULKAN_PIPELINE_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDER_VULKAN_PIPELINE_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/handle.h"
#include "comet/rendering/driver/vulkan/data/vulkan_pipeline.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_pass_handler.h"

namespace comet {
namespace rendering {
namespace vk {

struct PipelineHandlerDescr : HandlerDescr {
  const RenderPassHandler* render_pass_handler{nullptr};
};

class PipelineHandler : public Handler {
 public:
  PipelineHandler() = delete;
  explicit PipelineHandler(const PipelineHandlerDescr& descr);
  PipelineHandler(const PipelineHandler&) = delete;
  PipelineHandler(PipelineHandler&&) = delete;
  PipelineHandler& operator=(const PipelineHandler&) = delete;
  PipelineHandler& operator=(PipelineHandler&&) = delete;
  ~PipelineHandler() override = default;

  PipelineLayoutHandle GenerateLayout(const PipelineLayoutDescr& descr);
  PipelineHandle Generate(const GraphicsPipelineDescr& descr);
  PipelineHandle Generate(const ComputePipelineDescr& descr);

  void DestroyLayout(PipelineLayoutHandle handle);
  void Destroy(PipelineHandle handle);

  void Bind(PipelineHandle handle);

  void Reset();

  VkPipelineLayout GetNativeLayoutHandle(PipelineHandle handle) const;
  VkPipelineLayout GetNativeLayoutHandle(PipelineLayoutHandle handle) const;
  VkPipeline GetNativeHandle(PipelineHandle handle) const;
  PipelineBindType GetBindType(PipelineHandle handle) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  Pipeline* Get(PipelineHandle handle);
  const Pipeline* Get(PipelineHandle handle) const;
  Pipeline* TryGet(PipelineHandle handle);
  const Pipeline* TryGet(PipelineHandle handle) const;

  PipelineLayout* GetLayout(PipelineLayoutHandle handle);
  const PipelineLayout* GetLayout(PipelineLayoutHandle handle) const;
  PipelineLayout* TryGetLayout(PipelineLayoutHandle handle);
  const PipelineLayout* TryGetLayout(PipelineLayoutHandle handle) const;

  void DestroyPipelineObject(Pipeline* pipeline);
  void DestroyPipelineLayoutObject(PipelineLayout* layout);

  memory::FiberFreeListAllocator allocator_{
      math::Max(sizeof(Pipeline), sizeof(PipelineLayout)), 256,
      memory::kEngineMemoryTagRendering};

  HandlePool<PipelineHandleTag> pipeline_pool_{};
  HandlePool<PipelineLayoutHandleTag> layout_pool_{};

  Array<Pipeline*> pipelines_{};
  Array<PipelineLayout*> layouts_{};

  PipelineHandle bound_pipeline_{};

  const RenderPassHandler* render_pass_handler_{nullptr};
};

}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDER_VULKAN_PIPELINE_HANDLER_H_