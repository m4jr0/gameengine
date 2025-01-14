// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_VIEW_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_VIEW_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/data/vulkan_render_pass.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_material_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_pipeline_handler.h"
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
  PipelineHandler* pipeline_handler{nullptr};
  RenderPassHandler* render_pass_handler{nullptr};
  RenderProxyHandler* render_proxy_handler{nullptr};
#ifdef COMET_DEBUG
  DebuggerDisplayerManager* debugger_displayer_manager{nullptr};
#endif  // COMET_DEBUG
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
  virtual ~ViewHandler() = default;

  void Initialize() override;
  void Shutdown() override;
  void Destroy(usize view);
  void Destroy(View* view);
  void Update(frame::FramePacket* packet);
  void SetSize(WindowSize width, WindowSize height);

  const View* Get(usize index) const;
  const View* TryGet(usize index) const;
  const View* Generate(const RenderingViewDescr& descr);

 private:
  View* Get(usize index);
  View* TryGet(usize index);
  void Destroy(View* view, bool is_destroying_handler);

  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagRendering};
  Array<std::unique_ptr<View>> views_{};
  ShaderHandler* shader_handler_{nullptr};
  PipelineHandler* pipeline_handler_{nullptr};
  RenderPassHandler* render_pass_handler_{nullptr};
  RenderProxyHandler* render_proxy_handler_{nullptr};
  VulkanGlfwWindow* window_{nullptr};
  Array<RenderingViewDescr>* rendering_view_descrs_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_VIEW_HANDLER_H_
