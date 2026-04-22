// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_VIEW_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_VIEW_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_lighting_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_mesh_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_pass_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_proxy_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_texture_handler.h"
#include "comet/rendering/driver/vulkan/view/vulkan_view.h"
#include "comet/rendering/type/common.h"
#include "comet/rendering/type/light.h"
#include "comet/rendering/type/view.h"
#include "comet/rendering/window/glfw/vulkan/vulkan_glfw_window.h"

namespace comet {
namespace rendering {
namespace vk {
struct ViewHandlerDescr : HandlerDescr {
  const ShadowSettings* shadow_settings{nullptr};
  ShaderHandler* shader_handler{nullptr};
  TextureHandler* texture_handler{nullptr};
  RenderPassHandler* render_pass_handler{nullptr};
  RenderProxyHandler* render_proxy_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
  LightingHandler* lighting_handler{nullptr};
  VulkanGlfwWindow* window{nullptr};
  Array<RenderingViewDescr>* rendering_view_descrs{nullptr};
};

class ViewHandler : public Handler {
 public:
  ViewHandler() = delete;
  explicit ViewHandler(const ViewHandlerDescr& descr);
  ViewHandler(const ViewHandler&) = delete;
  ViewHandler(ViewHandler&&) = delete;
  ViewHandler& operator=(const ViewHandler&) = delete;
  ViewHandler& operator=(ViewHandler&&) = delete;
  ~ViewHandler() override = default;

  void Update(frame::FramePacket* packet);

  const View* Generate(const RenderingViewDescr& descr);
  void Destroy(usize view);
  void Destroy(View* view);

  void SetSize(WindowSize width, WindowSize height);

  const View* Get(usize index) const;
  const View* TryGet(usize index) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  View* Get(usize index);
  View* TryGet(usize index);

  void Destroy(View* view, bool is_destroying_handler);

  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagRendering};
  Array<memory::UniquePtr<View>> views_{};
  const ShadowSettings* shadow_settings_{nullptr};
  ShaderHandler* shader_handler_{nullptr};
  TextureHandler* texture_handler_{nullptr};
  RenderPassHandler* render_pass_handler_{nullptr};
  RenderProxyHandler* render_proxy_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  LightingHandler* lighting_handler_{nullptr};
  VulkanGlfwWindow* window_{nullptr};
  Array<RenderingViewDescr>* rendering_view_descrs_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_VIEW_HANDLER_H_
