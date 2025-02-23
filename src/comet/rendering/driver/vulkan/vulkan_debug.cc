// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

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

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
namespace internal {
static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT{nullptr};
static VkDevice device_handle{VK_NULL_HANDLE};
}  // namespace internal

void InitializeDebugLabels(VkInstance instance_handle, VkDevice device_handle) {
  if (internal::vkSetDebugUtilsObjectNameEXT != nullptr) {
    return;
  }

  internal::device_handle = device_handle;

  internal::vkSetDebugUtilsObjectNameEXT =
      (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(
          instance_handle, "vkSetDebugUtilsObjectNameEXT");

  COMET_ASSERT(internal::vkSetDebugUtilsObjectNameEXT != nullptr,
               "Cound not load vkSetDebugUtilsObjectNameEXT!");
}

void SetDebugLabel(VkObjectType object_type, u64 object_handle,
                   const schar* label) {
  COMET_ASSERT(internal::vkSetDebugUtilsObjectNameEXT != nullptr,
               "Debug names are not initialized!");

  VkDebugUtilsObjectNameInfoEXT name_info = {};
  name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
  name_info.objectType = object_type;
  name_info.objectHandle = object_handle;
  name_info.pObjectName = label;
  internal::vkSetDebugUtilsObjectNameEXT(internal::device_handle, &name_info);
}

void SetDebugLabel(VkCommandBuffer command_buffer_handle, const schar* label) {
  SetDebugLabel(VK_OBJECT_TYPE_COMMAND_BUFFER,
                reinterpret_cast<uint64_t>(command_buffer_handle), label);
}

void SetDebugLabel(VkQueue queue_handle, const schar* label) {
  SetDebugLabel(VK_OBJECT_TYPE_QUEUE, reinterpret_cast<uint64_t>(queue_handle),
                label);
}

void SetDebugLabel(VkRenderPass render_pass_handle, const schar* label) {
  SetDebugLabel(VK_OBJECT_TYPE_RENDER_PASS,
                reinterpret_cast<uint64_t>(render_pass_handle), label);
}

void SetDebugLabel(VkImage image_handle, const schar* label) {
  SetDebugLabel(VK_OBJECT_TYPE_IMAGE, reinterpret_cast<uint64_t>(image_handle),
                label);
}

void SetDebugLabel(VkBuffer buffer_handle, const schar* label) {
  SetDebugLabel(VK_OBJECT_TYPE_BUFFER,
                reinterpret_cast<uint64_t>(buffer_handle), label);
}
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
}  // namespace debug
}  // namespace vk
}  // namespace rendering
}  // namespace comet
