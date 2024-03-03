// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
namespace debug {
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance_handle,
    const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* messenger) {
  const auto func{reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance_handle,
                            "vkCreateDebugUtilsMessengerEXT"))};

  if (func == nullptr) {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }

  return func(instance_handle, create_info, allocator, messenger);
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance_handle,
                                   VkDebugUtilsMessengerEXT messenger,
                                   const VkAllocationCallbacks* allocator) {
  const auto func{reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance_handle,
                            "vkDestroyDebugUtilsMessengerEXT"))};

  if (func == nullptr) {
    return;
  }

  func(instance_handle, messenger, allocator);
}

VkResult CreateDebugReportCallback(const VkInstance instance_handle,
                                   const VkDebugReportFlagsEXT flags,
                                   const PFN_vkDebugReportCallbackEXT callback,
                                   VkDebugReportCallbackEXT& report_callback) {
  auto func{reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
      vkGetInstanceProcAddr(instance_handle,
                            "vkCreateDebugReportCallbackEXT"))};

  if (func == nullptr) {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }

  VkDebugReportCallbackCreateInfoEXT create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
  create_info.pNext = VK_NULL_HANDLE;
  create_info.flags = flags;
  create_info.pfnCallback = callback;
  create_info.pUserData = VK_NULL_HANDLE;

  return func(instance_handle, &create_info, VK_NULL_HANDLE, &report_callback);
}

void DestroyDebugReportCallback(
    const VkInstance instance_handle,
    const VkDebugReportCallbackEXT report_callback) {
  const auto func{reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
      vkGetInstanceProcAddr(instance_handle,
                            "vkDestroyDebugReportCallbackEXT"))};

  if (func == nullptr) {
    return;
  }

  func(instance_handle, report_callback, nullptr);
}
}  // namespace debug
}  // namespace vk
}  // namespace rendering
}  // namespace comet
