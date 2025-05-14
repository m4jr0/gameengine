// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_SHADER_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_SHADER_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_pipeline_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/rendering/driver/vulkan/view/vulkan_view.h"

namespace comet {
namespace rendering {
namespace vk {
struct ShaderViewDescr : ViewDescr {
  ShaderHandler* shader_handler{nullptr};
  PipelineHandler* pipeline_handler{nullptr};
};

class ShaderView : public View {
 public:
  explicit ShaderView(const ShaderViewDescr& descr);
  ShaderView(const ShaderView&) = delete;
  ShaderView(ShaderView&&) = delete;
  ShaderView& operator=(const ShaderView&) = delete;
  ShaderView& operator=(ShaderView&&) = delete;
  virtual ~ShaderView() = default;

  virtual void Destroy() override;
  virtual void Update(frame::FramePacket*) = 0;

 protected:
  Shader* shader_{nullptr};
  ShaderHandler* shader_handler_{nullptr};
  PipelineHandler* pipeline_handler_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_SHADER_VIEW_H_
