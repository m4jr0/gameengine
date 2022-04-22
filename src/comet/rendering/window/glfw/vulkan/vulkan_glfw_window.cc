// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_glfw_window.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace rendering {
namespace vk {
VulkanGlfwWindow::VulkanGlfwWindow(WindowDescr& descr) : GlfwWindow(descr) {}

VulkanGlfwWindow::VulkanGlfwWindow(const VulkanGlfwWindow& other)
    : GlfwWindow(other) {}

VulkanGlfwWindow::VulkanGlfwWindow(VulkanGlfwWindow&& other) noexcept
    : GlfwWindow(std::move(other)) {}

VulkanGlfwWindow& VulkanGlfwWindow::operator=(const VulkanGlfwWindow& other) {
  if (this == &other) {
    return *this;
  }

  GlfwWindow::operator=(other);
  return *this;
}

VulkanGlfwWindow& VulkanGlfwWindow::operator=(
    VulkanGlfwWindow&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  GlfwWindow::operator=(std::move(other));
  return *this;
}

void VulkanGlfwWindow::InitializeSurface(VkInstance instance,
                                         VkSurfaceKHR& surface) {
  if (glfwCreateWindowSurface(instance, handle_, nullptr, &surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface");
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
