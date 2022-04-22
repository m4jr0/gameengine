// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_UTILS_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/vulkan_types.h"

namespace comet {
namespace rendering {
namespace vk {
namespace utils {
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* messenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT messenger,
                                   const VkAllocationCallbacks* allocator);
bool AreDeviceExtensionAvailable(VkPhysicalDevice device,
                                 const std::vector<const char*> extensions);
VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice physical_device);
bool HasStencilComponent(VkFormat format);
}  // namespace utils
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_UTILS_H_
