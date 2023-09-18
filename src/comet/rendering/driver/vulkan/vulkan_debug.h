// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DEBUG_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DEBUG_H_

#include "vulkan/vulkan.h"

namespace comet {
namespace rendering {
namespace vk {
namespace debug {
#define COMET_VULKAN_ABORT_ON_ERROR

#ifndef COMET_DEBUG
#define COMET_CHECK_VK(vk_expression, ...) vk_expression
#else
// Disable Vulkan's validation layers even in debug mode, if necessary.
#define COMET_CHECK_VK(vk_expression, ...)                  \
  do {                                                      \
    COMET_ASSERT(vk_expression == VK_SUCCESS, __VA_ARGS__); \
  } while (false)
#endif  // !COMET_DEBUG

#ifdef COMET_VULKAN_DEBUG_MODE
#undef VMA_DEBUG_LOG

#ifdef COMET_VULKAN_DEBUG_VMA
// Print debug messages from VMA.
#define VMA_DEBUG_LOG(format, ...)                                          \
  do {                                                                      \
    constexpr auto kMessageLength{255};                                     \
    char message[kMessageLength]{'\0'};                                     \
    const auto len{                                                         \
        std::snprintf(message, kMessageLength - 1, format, ##__VA_ARGS__)}; \
    message[len] = '\0';                                                    \
    COMET_LOG_RENDERING_DEBUG("[VMA] ", message);                           \
  } while (false)
#endif  // COMET_VULKAN_DEBUG_VMA
#endif  // COMET_VULKAN_DEBUG_MODE

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance_handle,
    const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* messenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance_handle,
                                   VkDebugUtilsMessengerEXT messenger,
                                   const VkAllocationCallbacks* allocator);
VkResult CreateDebugReportCallback(const VkInstance instance_handle,
                                   const VkDebugReportFlagsEXT flags,
                                   const PFN_vkDebugReportCallbackEXT callback,
                                   VkDebugReportCallbackEXT& report_callback);
void DestroyDebugReportCallback(const VkInstance instance_handle,
                                const VkDebugReportCallbackEXT report_callback);
}  // namespace debug
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DEBUG_H_
