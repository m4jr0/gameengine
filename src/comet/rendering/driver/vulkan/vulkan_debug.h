// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DEBUG_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DEBUG_H_

#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"

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

#ifdef COMET_RENDERING_DRIVER_DEBUG_MODE
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
#endif  // COMET_RENDERING_DRIVER_DEBUG_MODE

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

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
namespace internal {
static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT{nullptr};
static VkDevice device_handle{VK_NULL_HANDLE};
}  // namespace internal

void InitializeDebugLabels(VkInstance instance_handle, VkDevice device_handle);
void SetDebugLabel(VkObjectType object_type, u64 object_handle,
                   const schar* label);
void SetDebugLabel(VkCommandBuffer command_buffer_handle, const schar* label);
void SetDebugLabel(VkQueue queue_handle, const schar* label);
void SetDebugLabel(VkRenderPass render_pass_handle, const schar* label);
void SetDebugLabel(VkImage image_handle, const schar* label);
void SetDebugLabel(VkBuffer buffer_handle, const schar* label);
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
}  // namespace debug
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
#define COMET_VK_INITIALIZE_DEBUG_LABELS(instance_handle, device_handle) \
  comet::rendering::vk::debug::InitializeDebugLabels(instance_handle,    \
                                                     device_handle)
#define COMET_VK_SET_DEBUG_LABEL(object_handle, label) \
  comet::rendering::vk::debug::SetDebugLabel(object_handle, label)
#else
#define COMET_VK_INITIALIZE_DEBUG_LABELS(instance_handle, device_handle)
#define COMET_VK_SET_DEBUG_LABEL(object_handle, label)
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DEBUG_H_
