// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_WORLD_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_WORLD_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_proxy_handler.h"
#include "comet/rendering/driver/vulkan/view/vulkan_shader_view.h"

namespace comet {
namespace rendering {
namespace vk {
struct WorldViewDescr : ShaderViewDescr {
  RenderProxyHandler* render_proxy_handler{nullptr};
};

class WorldView : public ShaderView {
 public:
  explicit WorldView(const WorldViewDescr& descr);
  WorldView(const WorldView&) = delete;
  WorldView(WorldView&&) = delete;
  WorldView& operator=(const WorldView&) = delete;
  WorldView& operator=(WorldView&&) = delete;
  virtual ~WorldView() = default;

  void Initialize() override;
  void Destroy() override;
  void Update(frame::FramePacket* packet) override;

 private:
  RenderProxyHandler* render_proxy_handler_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_WORLD_VIEW_H_
