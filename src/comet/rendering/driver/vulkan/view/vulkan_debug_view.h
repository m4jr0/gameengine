// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_DEBUG_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_DEBUG_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_proxy_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/rendering/driver/vulkan/view/vulkan_view.h"
#include "comet/rendering/rendering_handle.h"

namespace comet {
namespace rendering {
namespace vk {
struct DebugViewDescr : ViewDescr {
  ShaderHandler* shader_handler{nullptr};
  RenderProxyHandler* render_proxy_handler{nullptr};
};

class DebugView : public View {
 public:
  explicit DebugView(const DebugViewDescr& descr);
  DebugView(const DebugView&) = delete;
  DebugView(DebugView&&) = delete;
  DebugView& operator=(const DebugView&) = delete;
  DebugView& operator=(DebugView&&) = delete;
  ~DebugView() override = default;

  void Update(frame::FramePacket* packet) override;

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  void UpdateDebugShader(const frame::FramePacket* packet);
  void RunDebugCullGeneration();
  void DrawDebugCull();

  void SetViewportAndScissor() const;

  ShaderHandler* shader_handler_{nullptr};
  RenderProxyHandler* render_proxy_handler_{nullptr};
  ShaderHandle debug_shader_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_DEBUG_VIEW_H_