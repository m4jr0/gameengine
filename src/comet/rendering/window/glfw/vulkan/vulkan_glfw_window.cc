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
    : GlfwWindow{std::move(other)}, surface_handle_{other.surface_handle_} {
  other.surface_handle_ = VK_NULL_HANDLE;
}

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
  surface_handle_ = other.surface_handle_;
  other.surface_handle_ = VK_NULL_HANDLE;
  return *this;
}

void VulkanGlfwWindow::AttachSurface(VkInstance instance_handle) {
  COMET_CHECK_VK(glfwCreateWindowSurface(instance_handle, handle_, nullptr,
                                         &surface_handle_),
                 "Failed to create window surface");
}

void VulkanGlfwWindow::DetachSurface(VkInstance instance_handle) {
  COMET_ASSERT(instance_handle != VK_NULL_HANDLE,
               "Trying to detach surface, but Vulkan instance is null!");

  if (surface_handle_ == VK_NULL_HANDLE) {
    return;
  }

  vkDestroySurfaceKHR(instance_handle, surface_handle_, VK_NULL_HANDLE);
  surface_handle_ = VK_NULL_HANDLE;
}

VulkanGlfwWindow::operator VkSurfaceKHR() const noexcept {
  return surface_handle_;
}

VkSurfaceKHR VulkanGlfwWindow::GetSurfaceHandle() const noexcept {
  return surface_handle_;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
