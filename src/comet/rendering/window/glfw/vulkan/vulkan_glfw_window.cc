// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_glfw_window.h"

#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
VulkanGlfwWindow::VulkanGlfwWindow(WindowDescr& descr) : GlfwWindow(descr) {}

VulkanGlfwWindow::VulkanGlfwWindow(const VulkanGlfwWindow& other)
    : GlfwWindow{other} {}

VulkanGlfwWindow::VulkanGlfwWindow(VulkanGlfwWindow&& other) noexcept
    : GlfwWindow{std::move(other)} {}

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

void VulkanGlfwWindow::AttachSurface(VkInstance instance) {
  COMET_CHECK_VK(glfwCreateWindowSurface(instance, handle_, nullptr, &surface_),
                 "Failed to create window surface");
}

void VulkanGlfwWindow::DetachSurface(VkInstance instance) {
  COMET_ASSERT(instance != VK_NULL_HANDLE,
               "Trying to detach surface, but Vulkan instance is null!");

  if (surface_ == VK_NULL_HANDLE) {
    return;
  }

  vkDestroySurfaceKHR(instance, surface_, VK_NULL_HANDLE);
  surface_ = VK_NULL_HANDLE;
}

VulkanGlfwWindow::operator VkSurfaceKHR() const noexcept { return surface_; }

VkSurfaceKHR VulkanGlfwWindow::GetSurface() const noexcept { return surface_; }
}  // namespace vk
}  // namespace rendering
}  // namespace comet
