// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_VIEW_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_VIEW_HANDLER_H_

#include "comet_precompile.h"

#include "comet/rendering/driver/vulkan/data/vulkan_render_pass.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_material_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_pass_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_proxy_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/rendering/driver/vulkan/view/vulkan_view.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/rendering_common.h"
#include "comet/rendering/window/glfw/vulkan/vulkan_glfw_window.h"

namespace comet {
namespace rendering {
namespace vk {
struct ViewHandlerDescr : HandlerDescr {
  ShaderHandler* shader_handler{nullptr};
  RenderPassHandler* render_pass_handler{nullptr};
  RenderProxyHandler* render_proxy_handler{nullptr};
  VulkanGlfwWindow* window{nullptr};
  std::vector<RenderingViewDescr>* rendering_view_descrs{nullptr};
};

class ViewHandler : public Handler {
 public:
  ViewHandler() = delete;
  explicit ViewHandler(const ViewHandlerDescr& descr);
  ViewHandler(const ViewHandler&) = delete;
  ViewHandler(ViewHandler&& other) = delete;
  ViewHandler& operator=(const ViewHandler&) = delete;
  ViewHandler& operator=(ViewHandler&& other) = delete;
  virtual ~ViewHandler() = default;

  void Initialize() override;
  void Shutdown() override;
  void Destroy(uindex view);
  void Destroy(View& view);
  void Update(const ViewPacket& packet);
  void SetSize(WindowSize width, WindowSize height);

  const View* Get(uindex index) const;
  const View* TryGet(uindex index) const;
  const View* Generate(const RenderingViewDescr& descr);

 private:
  View* Get(uindex index);
  View* TryGet(uindex index);
  void Destroy(View& view, bool is_destroying_handler);

  std::vector<std::unique_ptr<View>> views_{};
  ShaderHandler* shader_handler_{nullptr};
  RenderPassHandler* render_pass_handler_{nullptr};
  RenderProxyHandler* render_proxy_handler_{nullptr};
  VulkanGlfwWindow* window_{nullptr};
  std::vector<RenderingViewDescr>* rendering_view_descrs_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_VIEW_HANDLER_H_
