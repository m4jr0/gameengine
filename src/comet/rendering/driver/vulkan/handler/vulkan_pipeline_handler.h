// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDER_VULKAN_PIPELINE_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDER_VULKAN_PIPELINE_HANDLER_H_

#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/map.h"
#include "comet/rendering/driver/vulkan/data/vulkan_pipeline.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"

namespace comet {
namespace rendering {
namespace vk {
using PipelineHandlerDescr = HandlerDescr;

class PipelineHandler : public Handler {
 public:
  PipelineHandler() = delete;
  explicit PipelineHandler(const PipelineHandlerDescr& descr);
  PipelineHandler(const PipelineHandler&) = delete;
  PipelineHandler(PipelineHandler&&) = delete;
  PipelineHandler& operator=(const PipelineHandler&) = delete;
  PipelineHandler& operator=(PipelineHandler&&) = delete;
  virtual ~PipelineHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  const PipelineLayout* GenerateLayout(const PipelineLayoutDescr& descr);
  const Pipeline* Generate(const GraphicsPipelineDescr& descr);
  const Pipeline* Generate(const ComputePipelineDescr& descr);
  void DestroyLayout(PipelineLayoutId pipeline_layout_id);
  void DestroyLayout(PipelineLayout* pipeline_layout);
  void Destroy(PipelineId pipeline_id);
  void Destroy(Pipeline* pipeline);
  void Bind(const Pipeline* pipeline);
  void Reset();

 private:
  static inline PipelineId pipeline_id_counter_{0};
  static inline PipelineLayoutId pipeline_layout_id_counter_{0};

  Pipeline* Get(PipelineId pipeline_id);
  Pipeline* TryGet(PipelineId pipeline_id);
  PipelineLayout* GetLayout(PipelineLayoutId pipeline_layout_id);
  PipelineLayout* TryGetLayout(PipelineLayoutId pipeline_layout_id);
  void Destroy(Pipeline* pipeline, bool is_destroying_handler);
  void DestroyLayout(PipelineLayout* pipeline_layout,
                     bool is_destroying_handler);

  memory::FiberStackAllocator allocator_{
      math::Max(sizeof(Pair<PipelineId, Pipeline>),
                sizeof(Pair<PipelineLayoutId, PipelineLayout>)),
      256, memory::kEngineMemoryTagRendering};
  Map<PipelineId, Pipeline*> pipelines_{};
  Map<PipelineLayoutId, PipelineLayout*> pipeline_layouts_{};
  const Pipeline* bound_pipeline_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDER_VULKAN_PIPELINE_HANDLER_H_
