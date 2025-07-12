// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_device.h"

#include <type_traits>

#include "comet/core/c_string.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/logger.h"
#include "comet/core/type/array.h"
#include "comet/core/type/ordered_set.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_alloc.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
bool AreQueueFamilyIndicesComplete(const QueueFamilyIndices& indices) {
  return indices.graphics_family.has_value() &&
         indices.present_family.has_value() &&
         indices.transfer_family.has_value();
}

bool IsTransferFamilyInQueueFamilyIndices(const QueueFamilyIndices& indices) {
  if (!indices.graphics_family.has_value() ||
      !indices.transfer_family.has_value()) {
    return false;
  }

  return indices.graphics_family.value() != indices.transfer_family.value();
}

frame::FrameArray<u32> GetUniqueIndices(const QueueFamilyIndices& indices) {
  COMET_ASSERT(AreQueueFamilyIndicesComplete(indices),
               "Queue family indices are not complete");

  frame::FrameOrderedSet<u32> set{};
  set.Reserve(3);
  set.Add(indices.graphics_family.value());
  set.Add(indices.present_family.value());
  set.Add(indices.transfer_family.value());

  frame::FrameArray<u32> list{};
  list.Reserve(set.GetSize());

  for (auto& index : set) {
    list.PushBack(index);
  }

  return list;
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physical_device_handle,
                                     VkSurfaceKHR surface_handle) {
  QueueFamilyIndices indices{};

  u32 queue_family_count{0};
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device_handle,
                                           &queue_family_count, VK_NULL_HANDLE);

  if (queue_family_count == 0) {
    return indices;
  }

  frame::FrameArray<VkQueueFamilyProperties> queue_families{};
  queue_families.Resize(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(
      physical_device_handle, &queue_family_count, queue_families.GetData());

  u32 queue_index{0};

  for (const auto& queue_family : queue_families) {
    VkBool32 is_present_support;
    COMET_CHECK_VK(vkGetPhysicalDeviceSurfaceSupportKHR(
                       physical_device_handle, queue_index, surface_handle,
                       &is_present_support),
                   "Unable to get physical device surface support!");
    // At first, we explicitly try to find a queue family specialized for
    // transfer operations.
    if (kIsSpecificTransferQueue && !indices.transfer_family.has_value() &&
        (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 &&
        queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
      indices.transfer_family = queue_index;
    }

    if (!indices.graphics_family.has_value() &&
        queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = queue_index;

      if (!indices.transfer_family.has_value() && !kIsSpecificTransferQueue) {
        indices.transfer_family = queue_index;
      }
    }

    if (!indices.present_family.has_value() && is_present_support) {
      indices.present_family = queue_index;
    }

    if (AreQueueFamilyIndicesComplete(indices)) {
      break;
    }

    ++queue_index;
  }

  // If no specific queue is found, fall back to the graphics queue family, if
  // possible.
  if (!indices.transfer_family.has_value() &&
      indices.graphics_family.has_value()) {
    indices.transfer_family = indices.graphics_family.value();
  }

  return indices;
}

VkSampleCountFlagBits GetMaxUsableSampleCount(
    VkPhysicalDevice physical_device_handle) {
  VkPhysicalDeviceProperties physical_device_properties;
  vkGetPhysicalDeviceProperties(physical_device_handle,
                                &physical_device_properties);

  auto counts{physical_device_properties.limits.framebufferColorSampleCounts &
              physical_device_properties.limits.framebufferDepthSampleCounts};

  if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
  if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
  if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
  if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
  if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
  if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

  return VK_SAMPLE_COUNT_1_BIT;
}

Device::Device(const DeviceDescr& descr)
    : is_sampler_anisotropy_{descr.is_sampler_anisotropy},
      is_sample_rate_shading_{descr.is_sample_rate_shading},
      anti_aliasing_type_{descr.anti_aliasing_type},
      instance_handle_{descr.instance_handle},
      surface_handle_{descr.surface_handle} {
  COMET_ASSERT(instance_handle_ != VK_NULL_HANDLE,
               "Instance handle provided is null!");

  COMET_ASSERT(surface_handle_ != VK_NULL_HANDLE,
               "Surface handle provided is null!");
}

Device::~Device() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for device, but it is still initialized!");
}

void Device::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize device, but it is already done!");
  ResolvePhysicalDeviceHandle();
  vkGetPhysicalDeviceProperties(physical_device_handle_, &properties_);
  vkGetPhysicalDeviceFeatures(physical_device_handle_, &features_);
  vkGetPhysicalDeviceMemoryProperties(physical_device_handle_,
                                      &memory_properties_);
  auto max_samples{GetMaxUsableSampleCount(physical_device_handle_)};

  switch (anti_aliasing_type_) {
    case AntiAliasingType::None:
      msaa_samples_ = VK_SAMPLE_COUNT_1_BIT;
      break;
    case AntiAliasingType::Msaa:
      msaa_samples_ = max_samples;
      break;
    case AntiAliasingType::MsaaX2:
      msaa_samples_ = VK_SAMPLE_COUNT_2_BIT;
      break;
    case AntiAliasingType::MsaaX4:
      msaa_samples_ = VK_SAMPLE_COUNT_4_BIT;
      break;
    case AntiAliasingType::MsaaX8:
      msaa_samples_ = VK_SAMPLE_COUNT_8_BIT;
      break;
    case AntiAliasingType::MsaaX16:
      msaa_samples_ = VK_SAMPLE_COUNT_16_BIT;
      break;
    case AntiAliasingType::MsaaX32:
      msaa_samples_ = VK_SAMPLE_COUNT_32_BIT;
      break;
    case AntiAliasingType::MsaaX64:
      msaa_samples_ = VK_SAMPLE_COUNT_64_BIT;
      break;
    default:
      COMET_LOG_RENDERING_ERROR(
          "Unsupported anti-aliasing type: ",
          static_cast<std::underlying_type_t<AntiAliasingType>>(
              anti_aliasing_type_),
          ". Setting it to none.");
      msaa_samples_ = VK_SAMPLE_COUNT_1_BIT;
      break;
  }

  auto max_samples_cast{static_cast<u32>(max_samples)};

  if (static_cast<u32>(msaa_samples_) > max_samples_cast) {
    COMET_LOG_RENDERING_ERROR(
        "Choosen MSAA (x",
        static_cast<std::underlying_type_t<AntiAliasingType>>(
            anti_aliasing_type_),
        ") is too high for current GPU. Setting it to x", max_samples_cast,
        ".");

    msaa_samples_ = max_samples;
  }

  COMET_LOG_RENDERING_DEBUG("Selected GPU: ", properties_.deviceName, ".");
  COMET_LOG_RENDERING_DEBUG("\t- Minimum alignment: ",
                            properties_.limits.minUniformBufferOffsetAlignment);

  u32 queue_family_count;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device_handle_,
                                           &queue_family_count, VK_NULL_HANDLE);
  COMET_ASSERT(queue_family_count > 0, "No queue family found!");

  queue_family_properties_ = Array<VkQueueFamilyProperties>{&allocator_};
  queue_family_properties_.Resize(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device_handle_,
                                           &queue_family_count,
                                           queue_family_properties_.GetData());

#ifdef COMET_DEBUG
  CheckRequiredExtensions();
#endif  // COMET_DEBUG

  COMET_ASSERT(AreDeviceExtensionsAvailable(physical_device_handle_),
               "At least one required extension is not supported!");

  queue_family_indices_ =
      FindQueueFamilies(physical_device_handle_, surface_handle_);

  auto unique_queue_family_indices{GetUniqueIndices(queue_family_indices_)};
  frame::FrameArray<VkDeviceQueueCreateInfo> queue_create_info{};
  queue_create_info.Reserve(unique_queue_family_indices.GetSize());
  auto queue_priority{1.0f};

  for (auto queue_family_index : unique_queue_family_indices) {
    queue_create_info.PushBack(init::GenerateDeviceQueueCreateInfo(
        queue_family_index, queue_priority));
  }

  VkPhysicalDeviceFeatures physical_device_features{};
  physical_device_features.samplerAnisotropy =
      is_sampler_anisotropy_ ? VK_TRUE : VK_FALSE;
  physical_device_features.sampleRateShading =
      is_sample_rate_shading_ ? VK_TRUE : VK_FALSE;
  physical_device_features.fillModeNonSolid = VK_TRUE;
  physical_device_features.multiDrawIndirect = VK_TRUE;

  VkPhysicalDeviceSynchronization2Features synchronization2_features{};
  synchronization2_features.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
  synchronization2_features.synchronization2 = VK_TRUE;
  synchronization2_features.pNext = VK_NULL_HANDLE;

  VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timeline_semaphore_features{};
  timeline_semaphore_features.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR;
  timeline_semaphore_features.timelineSemaphore = VK_TRUE;
  timeline_semaphore_features.pNext = &synchronization2_features;

  auto create_info{init::GenerateDeviceCreateInfo(
      queue_create_info, physical_device_features,
      kRequiredExtensions_.GetData(),
      static_cast<u32>(kRequiredExtensions_.GetSize()),
      &timeline_semaphore_features)};

  COMET_CHECK_VK(
      vkCreateDevice(physical_device_handle_, &create_info,
                     MemoryCallbacks::Get().GetAllocCallbacksHandle(),
                     &handle_),
      "Failed to create logical device!");

  vkGetDeviceQueue(handle_, queue_family_indices_.graphics_family.value(), 0,
                   &graphics_queue_handle_);

  vkGetDeviceQueue(handle_, queue_family_indices_.present_family.value(), 0,
                   &present_queue_handle_);

  if (!IsTransferFamilyInQueueFamilyIndices(queue_family_indices_)) {
    is_initialized_ = true;
    return;
  }

  vkGetDeviceQueue(handle_, queue_family_indices_.transfer_family.value(), 0,
                   &transfer_queue_handle_);
  COMET_ASSERT(transfer_queue_handle_ != VK_NULL_HANDLE,
               "Could not get transfer queue handle!");
  is_initialized_ = true;
}

void Device::Destroy() {
  COMET_ASSERT(is_initialized_,
               "Tried to destroy device, but it is not initialized!");
  if (handle_ != VK_NULL_HANDLE) {
    vkDestroyDevice(handle_, MemoryCallbacks::Get().GetAllocCallbacksHandle());
    handle_ = VK_NULL_HANDLE;
  }

  is_sampler_anisotropy_ = false;
  is_sample_rate_shading_ = false;
  anti_aliasing_type_ = AntiAliasingType::None;
  properties_ = {};
  features_ = {};
  memory_properties_ = {};
  queue_family_properties_.Destroy();
  queue_family_indices_ = {};
  instance_handle_ = VK_NULL_HANDLE;
  physical_device_handle_ = VK_NULL_HANDLE;
  handle_ = VK_NULL_HANDLE;
  surface_handle_ = VK_NULL_HANDLE;
  graphics_queue_handle_ = VK_NULL_HANDLE;
  present_queue_handle_ = VK_NULL_HANDLE;
  transfer_queue_handle_ = VK_NULL_HANDLE;
  msaa_samples_ = VK_SAMPLE_COUNT_1_BIT;

  is_initialized_ = false;
}

void Device::WaitIdle() const {
  if (handle_ == VK_NULL_HANDLE) {
    return;
  }

  vkDeviceWaitIdle(handle_);
}

VkFormat Device::ChooseFormat(VkImageTiling tiling,
                              VkFormatFeatureFlags features,
                              const Array<VkFormat>& candidates) const {
  for (auto format : candidates) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(physical_device_handle_, format,
                                        &properties);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (properties.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (properties.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  COMET_ASSERT(false, "Failed to find supported format");
  return VK_FORMAT_UNDEFINED;
}

VkFormat Device::ChooseDepthFormat() const {
  frame::FrameArray<VkFormat> candidates{VK_FORMAT_D32_SFLOAT,
                                         VK_FORMAT_D32_SFLOAT_S8_UINT,
                                         VK_FORMAT_D24_UNORM_S8_UINT};

  // TODO(m4jr0): Retrieve settings from configuration.
  return ChooseFormat(VK_IMAGE_TILING_OPTIMAL,
                      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      candidates);
}

VkDevice Device::GetHandle() const noexcept { return handle_; }

Device::operator VkDevice() const noexcept { return GetHandle(); };

VkPhysicalDevice Device::GetPhysicalDeviceHandle() const noexcept {
  return physical_device_handle_;
}

const VkPhysicalDeviceProperties& Device::GetProperties() const noexcept {
  return properties_;
}

const VkPhysicalDeviceFeatures& Device::GetFeatures() const noexcept {
  return features_;
}

const VkPhysicalDeviceMemoryProperties& Device::GetMemoryProperties()
    const noexcept {
  return memory_properties_;
}

VkSampleCountFlagBits Device::GetMsaaSamples() const noexcept {
  return msaa_samples_;
}

bool Device::IsMsaa() const noexcept {
  return (msaa_samples_ & VK_SAMPLE_COUNT_1_BIT) != VK_SAMPLE_COUNT_1_BIT;
}

bool Device::IsInitialized() const noexcept { return is_initialized_; }

const QueueFamilyIndices& Device::GetQueueFamilyIndices() const noexcept {
  return queue_family_indices_;
}

VkQueue Device::GetGraphicsQueueHandle() const noexcept {
  return graphics_queue_handle_;
}

VkQueue Device::GetPresentQueueHandle() const noexcept {
  return present_queue_handle_;
}

VkQueue Device::GetTransferQueueHandle() const noexcept {
  if (transfer_queue_handle_ != VK_NULL_HANDLE) {
    return transfer_queue_handle_;
  }

  return graphics_queue_handle_;
}

u32 Device::GetGraphicsQueueIndex() const noexcept {
  COMET_ASSERT(queue_family_indices_.graphics_family.has_value(),
               "No graphics family available!");
  return queue_family_indices_.graphics_family.value();
}

u32 Device::GetPresentQueueIndex() const noexcept {
  COMET_ASSERT(queue_family_indices_.present_family.has_value(),
               "No present family available!");
  return queue_family_indices_.present_family.value();
}

u32 Device::GetTransferQueueIndex() const noexcept {
  COMET_ASSERT(queue_family_indices_.transfer_family.has_value(),
               "No transfer family available!");
  return queue_family_indices_.transfer_family.value();
}

PhysicalDeviceScore Device::GetPhysicalDeviceScore(
    VkPhysicalDevice physical_device_handle) const {
  PhysicalDeviceScore score{0};

  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;

  vkGetPhysicalDeviceProperties(physical_device_handle, &properties);
  vkGetPhysicalDeviceFeatures(physical_device_handle, &features);

  // First, mandatory properties and features.
  if (!features.samplerAnisotropy) {
    return score;
  }

  if (!features.geometryShader) {
    return score;
  }

  if (!features.multiDrawIndirect) {
    return score;
  }

  if (!AreDeviceExtensionsAvailable(physical_device_handle)) {
    return score;
  }

  auto indices{FindQueueFamilies(physical_device_handle, surface_handle_)};

  if (!AreQueueFamilyIndicesComplete(indices)) {
    return score;
  }

  SwapchainSupportDetails swapchain_support_details{};
  swapchain_support_details.formats = Array<VkSurfaceFormatKHR>{&allocator_};
  swapchain_support_details.present_modes =
      Array<VkPresentModeKHR>{&allocator_};
  QuerySwapchainSupportDetails(physical_device_handle, surface_handle_,
                               swapchain_support_details);

  if (swapchain_support_details.formats.IsEmpty() ||
      swapchain_support_details.present_modes.IsEmpty()) {
    return score;
  }

  // Other properties and features.
  if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 1000;
  }

  const auto msaa_samples{GetMaxUsableSampleCount(physical_device_handle)};
  score += 5 * msaa_samples;
  score += properties.limits.maxImageDimension2D;

  return score;
}

bool Device::AreDeviceExtensionsAvailable(
    VkPhysicalDevice physical_device_handle) const {
  if (kRequiredExtensions_.IsEmpty()) {
    return true;
  }

  u32 extension_count;
  vkEnumerateDeviceExtensionProperties(physical_device_handle, VK_NULL_HANDLE,
                                       &extension_count, VK_NULL_HANDLE);

  if (extension_count == 0) {
    COMET_LOG_RENDERING_ERROR(
        "At least one extension is required, when there is none available on "
        "this device.");
    return false;
  }

  frame::FrameArray<VkExtensionProperties> extensions_properties{};
  extensions_properties.Resize(extension_count);

  COMET_CHECK_VK(vkEnumerateDeviceExtensionProperties(
                     physical_device_handle, VK_NULL_HANDLE, &extension_count,
                     extensions_properties.GetData()),
                 "Failed to enumerate physical device extension properties!");

  for (auto& extension_name : kRequiredExtensions_) {
    auto is_found{false};

    for (auto& extension_properties : extensions_properties) {
      if (AreStringsEqual(extension_name, extension_properties.extensionName)) {
        is_found = true;
        break;
      }
    }

    if (is_found) {
      continue;
    }

    COMET_LOG_RENDERING_ERROR(
        "Unsupported extension detected: ", extension_name, "!");
    return false;
  }

  return true;
}

void Device::ResolvePhysicalDeviceHandle() {
  COMET_ASSERT(instance_handle_ != VK_NULL_HANDLE,
               "Vulkan instance handle is null!");

  u32 physical_device_count{0};
  vkEnumeratePhysicalDevices(instance_handle_, &physical_device_count,
                             VK_NULL_HANDLE);
  COMET_ASSERT(physical_device_count != 0,
               "Failed to find GPUs with Vulkan support!");

  frame::FrameArray<VkPhysicalDevice> physical_device_handles{};
  physical_device_handles.Resize(physical_device_count);
  vkEnumeratePhysicalDevices(instance_handle_, &physical_device_count,
                             physical_device_handles.GetData());

  PhysicalDeviceScore best_score{0};

  for (const auto& physical_device_handle : physical_device_handles) {
    const auto score{GetPhysicalDeviceScore(physical_device_handle)};

    if (score > best_score || physical_device_handle_ == VK_NULL_HANDLE) {
      physical_device_handle_ = physical_device_handle;
      best_score = score;
    }
  }

  COMET_ASSERT(physical_device_handle_ != VK_NULL_HANDLE,
               "Failed to find a suitable GPU!");
}

#ifdef COMET_DEBUG
void Device::CheckRequiredExtensions() const {
  usize found_extensions_count{0};

  for (const auto* extension_name : kRequiredExtensions_) {
    for (const auto* to_check_name : kExtensionsToCheck_) {
      if (AreStringsEqual(extension_name, to_check_name)) {
        ++found_extensions_count;
        continue;
      }
    }
  }

  COMET_ASSERT(found_extensions_count >= kExtensionsToCheck_.GetSize(),
               "At least one mandatory extension is missing!");
}
#endif  // COMET_DEBUG
}  // namespace vk
}  // namespace rendering
}  // namespace comet
