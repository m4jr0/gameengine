// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_GLFW_VULKAN_VULKAN_GLFW_WINDOW_H_
#define COMET_COMET_RENDERING_WINDOW_GLFW_VULKAN_VULKAN_GLFW_WINDOW_H_

// External. ///////////////////////////////////////////////////////////////////

#define GLFW_INCLUDE_NONE
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/window/glfw/glfw_window.h"

namespace comet {
namespace rendering {
namespace vk {
class VulkanGlfwWindow : public GlfwWindow {
 public:
  VulkanGlfwWindow() = delete;
  explicit VulkanGlfwWindow(WindowDescr& descr);
  VulkanGlfwWindow(const VulkanGlfwWindow&);
  VulkanGlfwWindow(VulkanGlfwWindow&&) noexcept;
  VulkanGlfwWindow& operator=(const VulkanGlfwWindow&);
  VulkanGlfwWindow& operator=(VulkanGlfwWindow&&) noexcept;
  virtual ~VulkanGlfwWindow() = default;

  void AttachSurface(VkInstance instance_handle);
  void DetachSurface(VkInstance instance_handle);

  operator VkSurfaceKHR() const noexcept;

  VkSurfaceKHR GetSurfaceHandle() const noexcept;

 private:
  VkSurfaceKHR surface_handle_{VK_NULL_HANDLE};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_GLFW_VULKAN_VULKAN_GLFW_WINDOW_H_