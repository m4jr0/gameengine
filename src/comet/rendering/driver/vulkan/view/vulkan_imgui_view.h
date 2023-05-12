// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_IMGUI_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_IMGUI_VIEW_H_

#include "comet_precompile.h"

#ifdef COMET_IMGUI
#include "vulkan/vulkan.h"

#include "comet/physics/physics_manager.h"
#include "comet/rendering/driver/vulkan/view/vulkan_view.h"
#include "comet/rendering/rendering_manager.h"
#include "comet/rendering/window/glfw/vulkan/vulkan_glfw_window.h"

namespace comet {
namespace rendering {
namespace vk {
struct ImGuiViewDescr : ViewDescr {
  VulkanGlfwWindow* window{nullptr};
#ifdef COMET_DEBUG
  DebuggerDisplayerManager* debugger_displayer_manager{nullptr};
#endif  // COMET_DEBUG;
};

class ImGuiView : public View {
 public:
  explicit ImGuiView(const ImGuiViewDescr& descr);
  ImGuiView(const ImGuiView&) = delete;
  ImGuiView(ImGuiView&&) = delete;
  ImGuiView& operator=(const ImGuiView&) = delete;
  ImGuiView& operator=(ImGuiView&&) = delete;
  virtual ~ImGuiView() = default;

  void Initialize() override;
  void Destroy() override;
  void Update(const ViewPacket& packet) override;

 private:
  void Draw() const;

  VkDescriptorPool descriptor_pool_handle_{VK_NULL_HANDLE};
  VulkanGlfwWindow* window_{nullptr};
#ifdef COMET_DEBUG
  DebuggerDisplayerManager* debugger_displayer_manager_{nullptr};
#endif  // COMET_DEBUG
  physics::PhysicsManager* physics_manager_{nullptr};
  rendering::RenderingManager* rendering_manager_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_IMGUI_VIEW_H_
