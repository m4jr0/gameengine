// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* messenger) {
  const auto func{(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT")};

  if (func == nullptr) {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }

  return func(instance, create_info, allocator, messenger);
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT messenger,
                                   const VkAllocationCallbacks* allocator) {
  const auto func{(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT")};

  if (func == nullptr) {
    return;
  }

  func(instance, messenger, allocator);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
