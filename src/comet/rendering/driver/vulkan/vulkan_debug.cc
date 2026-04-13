// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_debug.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"

namespace comet {
namespace rendering {
namespace vk {
namespace debug {
const schar* GetVkResultString(VkResult result) {
  switch (result) {
    case VK_SUCCESS:
      return "VK_SUCCESS";
    case VK_NOT_READY:
      return "VK_NOT_READY";
    case VK_TIMEOUT:
      return "VK_TIMEOUT";
    case VK_EVENT_SET:
      return "VK_EVENT_SET";
    case VK_EVENT_RESET:
      return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
      return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
      return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
      return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
      return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
      return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
      return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
      return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
      return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
      return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN:
      return "VK_ERROR_UNKNOWN";
    case VK_ERROR_VALIDATION_FAILED:
      return "VK_ERROR_VALIDATION_FAILED";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
      return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
      return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
      return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_ERROR_FRAGMENTATION:
      return "VK_ERROR_FRAGMENTATION";
    case VK_PIPELINE_COMPILE_REQUIRED:
      return "VK_PIPELINE_COMPILE_REQUIRED";
    case VK_ERROR_NOT_PERMITTED:
      return "VK_ERROR_NOT_PERMITTED";
    case VK_ERROR_SURFACE_LOST_KHR:
      return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:
      return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
      return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_INVALID_SHADER_NV:
      return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
      return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
      return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
      return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
      return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
      return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
      return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
      return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT:
      return "VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
      return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR:
      return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR:
      return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR:
      return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR:
      return "VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
      return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
    case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
      return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
    case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
      return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
    case VK_PIPELINE_BINARY_MISSING_KHR:
      return "VK_PIPELINE_BINARY_MISSING_KHR";
    case VK_ERROR_NOT_ENOUGH_SPACE_KHR:
      return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
    default:
      return "???";
  }
}

const schar* GenerateTmpVkResultString(VkResult result) {
  return GenerateTmpFromFormat(96, "%s (%d)", GetVkResultString(result),
                               static_cast<s32>(result));
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance_handle,
    const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* messenger) {
  auto func{reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
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
  auto func{reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
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
  auto func{reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
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

  VkDebugUtilsObjectNameInfoEXT name_info{};
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

void SetDebugLabel(VkDescriptorSet descriptor_set_handle, const schar* label) {
  SetDebugLabel(VK_OBJECT_TYPE_DESCRIPTOR_SET,
                reinterpret_cast<uint64_t>(descriptor_set_handle), label);
}
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
}  // namespace debug
}  // namespace vk
}  // namespace rendering
}  // namespace comet
