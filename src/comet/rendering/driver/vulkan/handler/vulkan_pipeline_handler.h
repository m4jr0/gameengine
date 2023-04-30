// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDER_VULKAN_PIPELINE_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDER_VULKAN_PIPELINE_HANDLER_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

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

  void Shutdown() override;

  const Pipeline* Generate(const PipelineDescr& descr);
  const Pipeline* Get(PipelineId pipeline_id) const;
  const Pipeline* TryGet(PipelineId pipeline_id) const;
  const Pipeline* GetOrGenerate(const PipelineDescr& descr);
  void Destroy(PipelineId pipeline_id);
  void Destroy(Pipeline& pipeline);
  void Bind(const Pipeline& pipeline) const;

 private:
  const Pipeline* GenerateGraphics(const PipelineDescr& descr);
  Pipeline* Get(PipelineId pipeline_id);
  Pipeline* TryGet(PipelineId pipeline_id);
  void Destroy(Pipeline& pipeline, bool is_destroying_handler);

  std::unordered_map<PipelineId, Pipeline> pipelines_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDER_VULKAN_PIPELINE_HANDLER_H_
