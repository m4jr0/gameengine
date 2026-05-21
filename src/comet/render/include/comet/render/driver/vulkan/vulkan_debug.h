// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_VULKAN_DEBUG_H_
#define COMET_RENDER_DRIVER_VULKAN_VULKAN_DEBUG_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/logger/logging.h"

namespace comet {
namespace rendering {
namespace vk {
namespace debug {
const schar* GetVkResultString(VkResult result);

const schar* GenerateTmpVkResultString(VkResult result);

#define COMET_VULKAN_ABORT_ON_ERROR

#define VMA_LEAK_LOG_FORMAT(...)                                           \
  COMET_LOG_DEBUG(comet::LoggerType::Rendering,                            \
                  "vulkan_debug::internal::VmaLeakLog", "vma leak report", \
                  "message", comet::GenerateTmpFromFormat(1024, __VA_ARGS__))

#define VMA_ASSERT(cond) \
  COMET_ASSERT((cond), "vulkan_debug::internal::VmaAssert", "vma assert failed")

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
void InitializeDebugLabels(VkInstance instance_handle, VkDevice device_handle);

void SetDebugLabel(VkObjectType object_type, u64 object_handle,
                   const schar* label);
void SetDebugLabel(VkCommandBuffer command_buffer_handle, const schar* label);
void SetDebugLabel(VkQueue queue_handle, const schar* label);
void SetDebugLabel(VkRenderPass render_pass_handle, const schar* label);
void SetDebugLabel(VkImage image_handle, const schar* label);
void SetDebugLabel(VkBuffer buffer_handle, const schar* label);
void SetDebugLabel(VkDescriptorSet descriptor_set_handle, const schar* label);
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

#ifndef COMET_DEBUG
#define COMET_CHECK_VK(vk_expression, prefix, message, ...) \
  (static_cast<void>(vk_expression))
#else
#define COMET_CHECK_VK(vk_expression, prefix, message, ...)                   \
  do {                                                                        \
    const VkResult comet_vk_result{static_cast<VkResult>(vk_expression)};     \
    COMET_ASSERT(comet_vk_result == VK_SUCCESS, prefix, message, "vk_result", \
                 static_cast<s32>(comet_vk_result), "vk_result_str",          \
                 comet::rendering::vk::debug::GenerateTmpVkResultString(      \
                     comet_vk_result) __VA_OPT__(, ) __VA_ARGS__);            \
  } while (false)
#endif  // !COMET_DEBUG

#endif  // COMET_RENDER_DRIVER_VULKAN_VULKAN_DEBUG_H_
