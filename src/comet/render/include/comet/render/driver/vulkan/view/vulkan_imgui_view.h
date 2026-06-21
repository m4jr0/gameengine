// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_VIEW_VULKAN_IMGUI_VIEW_H_
#define COMET_RENDER_DRIVER_VULKAN_VIEW_VULKAN_IMGUI_VIEW_H_

#include "comet/core/essentials.h"

#ifdef COMET_IMGUI
// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/render/driver/vulkan/view/vulkan_view.h"
#include "comet/platform/window/glfw/vulkan/vulkan_glfw_window.h"

namespace comet {
namespace rendering {
namespace vk {
struct ImGuiViewDescr : ViewDescr {
  VulkanGlfwWindow* window{nullptr};
};

class ImGuiView : public View {
 public:
  explicit ImGuiView(const ImGuiViewDescr& descr);
  ImGuiView(const ImGuiView&) = delete;
  ImGuiView(ImGuiView&&) = delete;
  ImGuiView& operator=(const ImGuiView&) = delete;
  ImGuiView& operator=(ImGuiView&&) = delete;
  ~ImGuiView() override = default;

  void Prepare(const ViewUpdate&) override;
  void Begin(const ViewUpdate&) override;
  void Draw(const ViewUpdate&) override;
  void End(const ViewUpdate&) override;

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  void DrawDebugUi() const;

  VkDescriptorPool descriptor_pool_handle_{VK_NULL_HANDLE};
  VulkanGlfwWindow* window_{nullptr};
  bool is_pass_open_{false};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI

#endif  // COMET_RENDER_DRIVER_VULKAN_VIEW_VULKAN_IMGUI_VIEW_H_