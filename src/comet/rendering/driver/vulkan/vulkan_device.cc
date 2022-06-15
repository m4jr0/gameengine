// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_device.h"

#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/driver/vulkan/vulkan_initializers.h"

namespace comet {
namespace rendering {
namespace vk {
bool QueueFamilyIndices::IsComplete() const {
  return graphics_family.has_value() && present_family.has_value() &&
         transfer_family.has_value();
}

bool QueueFamilyIndices::IsSpecificTransferFamily() const {
  if (!graphics_family.has_value() || !transfer_family.has_value()) {
    return false;
  }

  return graphics_family.value() != transfer_family.value();
}

std::vector<u32> QueueFamilyIndices::GetUniqueIndices() const {
  COMET_ASSERT(IsComplete(), "Queue Family Indices is not complete");

  const auto graphics_fam_index{graphics_family.value()};
  const auto present_fam_index{present_family.value()};
  const auto transfer_fam_index{transfer_family.value()};

  std::set<u32> set{graphics_family.value(), present_family.value(),
                    transfer_family.value()};

  std::vector<u32> list{};
  list.reserve(set.size());

  for (auto it{set.begin()}; it != set.end();) {
    list.push_back(std::move(set.extract(it++).value()));
  }

  return list;
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physical_device,
                                     VkSurfaceKHR surface) {
  QueueFamilyIndices indices{};

  u32 queue_family_count{0};
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count,
                                           VK_NULL_HANDLE);

  if (queue_family_count == 0) {
    return indices;
  }

  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count,
                                           queue_families.data());

  u32 queue_index{0};

  for (const auto& queue_family : queue_families) {
    VkBool32 is_present_support;
    COMET_CHECK_VK(
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_index,
                                             surface, &is_present_support),
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

    if (indices.IsComplete()) {
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
    VkPhysicalDevice physical_device) {
  VkPhysicalDeviceProperties physical_device_properties;
  vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);

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

PhysicalDeviceScore GetPhysicalDeviceScore(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface,
    const std::vector<const char*>& required_extensions) {
  PhysicalDeviceScore score{0};

  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;

  vkGetPhysicalDeviceProperties(physical_device, &properties);
  vkGetPhysicalDeviceFeatures(physical_device, &features);

  // First, mandatory properties and features.
  if (!features.samplerAnisotropy) {
    return score;
  }

  if (!features.geometryShader) {
    return score;
  }

  if (!AreDeviceExtensionsAvailable(physical_device, required_extensions)) {
    return score;
  }

  auto indices{FindQueueFamilies(physical_device, surface)};

  if (!indices.IsComplete()) {
    return score;
  }

  auto swapchain_support_details{
      QuerySwapchainSupportDetails(physical_device, surface)};

  if (swapchain_support_details.formats.empty() ||
      swapchain_support_details.present_modes.empty()) {
    return score;
  }

  // Other properties and features.
  if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 1000;
  }

  const auto msaa_samples{GetMaxUsableSampleCount(physical_device)};
  score += 5 * msaa_samples;
  score += properties.limits.maxImageDimension2D;

  return score;
}

bool AreDeviceExtensionsAvailable(VkPhysicalDevice physical_device,
                                  const std::vector<const char*>& extensions) {
  if (extensions.empty()) {
    return true;
  }

  u32 extension_count;
  vkEnumerateDeviceExtensionProperties(physical_device, VK_NULL_HANDLE,
                                       &extension_count, VK_NULL_HANDLE);

  if (extension_count == 0) {
    COMET_LOG_RENDERING_ERROR(
        "At least one extension is required, when there is none available on "
        "this device.");
    return false;
  }

  std::vector<VkExtensionProperties> extensions_properties(extension_count);

  COMET_CHECK_VK(vkEnumerateDeviceExtensionProperties(
                     physical_device, VK_NULL_HANDLE, &extension_count,
                     extensions_properties.data()),
                 "Failed to enumerate physical device extension properties!");

  for (auto& extension_name : extensions) {
    bool is_found{false};

    for (auto& extension_properties : extensions_properties) {
      if (std::strncmp(extension_name, extension_properties.extensionName,
                       VK_MAX_EXTENSION_NAME_SIZE) == 0) {
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

VkPhysicalDevice GetBestPhysicalDevice(
    VkInstance instance, VkSurfaceKHR surface,
    const std::vector<const char*>& required_extensions) {
  COMET_ASSERT(instance != VK_NULL_HANDLE, "Vulkan instance is null!");

  u32 device_count{0};
  vkEnumeratePhysicalDevices(instance, &device_count, VK_NULL_HANDLE);
  COMET_ASSERT(device_count != 0, "Failed to find GPUs with Vulkan support!");
  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

  VkPhysicalDevice physical_device{VK_NULL_HANDLE};
  PhysicalDeviceScore best_score{0};

  for (const auto& device : devices) {
    const auto score{
        GetPhysicalDeviceScore(device, surface, required_extensions)};

    if (score > best_score || physical_device == VK_NULL_HANDLE) {
      physical_device = device;
    }
  }

  COMET_ASSERT(physical_device != VK_NULL_HANDLE,
               "Failed to find a suitable GPU!");

  return physical_device;
}

void VulkanDevice::Initialize(const VulkanDeviceDescr& descr) {
  COMET_ASSERT(descr.physical_device != VK_NULL_HANDLE,
               "Physical device provided is null!");

  COMET_ASSERT(descr.surface != VK_NULL_HANDLE, "Surface provided is null!");

  physical_device_ = descr.physical_device;

  vkGetPhysicalDeviceProperties(physical_device_, &properties_);
  vkGetPhysicalDeviceFeatures(physical_device_, &features_);
  vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties_);

  msaa_samples_ = GetMaxUsableSampleCount(physical_device_);

  COMET_LOG_RENDERING_DEBUG("Selected GPU: ", properties_.deviceName, ".");
  COMET_LOG_RENDERING_DEBUG("\t- Minimum alignment: ",
                            properties_.limits.minUniformBufferOffsetAlignment);

  u32 queue_family_count;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device_,
                                           &queue_family_count, VK_NULL_HANDLE);
  COMET_ASSERT(queue_family_count > 0, "No queue family found!");

  queue_family_properties_.resize(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(
      physical_device_, &queue_family_count, queue_family_properties_.data());

#ifdef COMET_DEBUG
  CheckRequiredExtensions();
#endif  // COMET_DEBUG

  COMET_ASSERT(
      AreDeviceExtensionsAvailable(physical_device_, required_extensions_),
      "At least one required extension is not supported!");

  queue_family_indices_ = FindQueueFamilies(physical_device_, descr.surface);

  auto unique_queue_family_indices{queue_family_indices_.GetUniqueIndices()};
  std::vector<VkDeviceQueueCreateInfo> queue_create_info{};
  auto queue_priority{1.0f};

  for (auto queue_family_index : unique_queue_family_indices) {
    queue_create_info.push_back(
        init::GetDeviceQueueCreateInfo(queue_family_index, queue_priority));
  }

  VkPhysicalDeviceFeatures device_features{};
  device_features.samplerAnisotropy =
      descr.is_sampler_anisotropy ? VK_TRUE : VK_FALSE;
  device_features.sampleRateShading =
      descr.is_sample_rate_shading ? VK_TRUE : VK_FALSE;
  device_features.fillModeNonSolid = VK_TRUE;

  auto create_info = init::GetDeviceCreateInfo(
      queue_create_info, device_features, required_extensions_);

  COMET_CHECK_VK(
      vkCreateDevice(physical_device_, &create_info, VK_NULL_HANDLE, &device_),
      "Failed to create logical device!");

  vkGetDeviceQueue(device_, queue_family_indices_.graphics_family.value(), 0,
                   &graphics_queue_);

  vkGetDeviceQueue(device_, queue_family_indices_.present_family.value(), 0,
                   &present_queue_);

  if (!queue_family_indices_.IsSpecificTransferFamily()) {
    return;
  }

  vkGetDeviceQueue(device_, queue_family_indices_.transfer_family.value(), 0,
                   &transfer_queue_);
}

void VulkanDevice::Destroy() {
  if (device_ != VK_NULL_HANDLE) {
    vkDestroyDevice(device_, VK_NULL_HANDLE);
    device_ = VK_NULL_HANDLE;
  }
}

VkFormat VulkanDevice::ChooseFormat(VkImageTiling tiling,
                                    VkFormatFeatureFlags features,
                                    const std::vector<VkFormat>& candidates) {
  for (auto format : candidates) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(physical_device_, format, &properties);

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

VkFormat VulkanDevice::ChooseDepthFormat() {
  // TODO(m4jr0): Retrieve settings from configuration.
  return ChooseFormat(VK_IMAGE_TILING_OPTIMAL,
                      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                       VK_FORMAT_D24_UNORM_S8_UINT});
}

VulkanDevice::operator VkDevice() const noexcept { return device_; };

VkDevice VulkanDevice::GetDevice() const noexcept { return device_; }

VkPhysicalDevice VulkanDevice::GetPhysicalDevice() const noexcept {
  return physical_device_;
}

const VkPhysicalDeviceProperties& VulkanDevice::GetProperties() const noexcept {
  return properties_;
}

const VkPhysicalDeviceFeatures& VulkanDevice::GetFeatures() const noexcept {
  return features_;
}

const VkPhysicalDeviceMemoryProperties& VulkanDevice::GetMemoryProperties()
    const noexcept {
  return memory_properties_;
}

VkSampleCountFlagBits VulkanDevice::GetMsaaSamples() const noexcept {
  return msaa_samples_;
}

const QueueFamilyIndices& VulkanDevice::GetQueueFamilyIndices() const noexcept {
  return queue_family_indices_;
}

VkQueue VulkanDevice::GetGraphicsQueue() const noexcept {
  return graphics_queue_;
}

VkQueue VulkanDevice::GetPresentQueue() const noexcept {
  return present_queue_;
}

VkQueue VulkanDevice::GetTransferQueue() const noexcept {
  if (transfer_queue_ != VK_NULL_HANDLE) {
    return transfer_queue_;
  }

  return graphics_queue_;
}

#ifdef COMET_DEBUG
void VulkanDevice::CheckRequiredExtensions() {
  uindex found_extensions_count{0};

  for (const auto* extension_name : required_extensions_) {
    for (const auto* to_check_name : kExtensionsToCheck_) {
      if (strncmp(extension_name, to_check_name, VK_MAX_EXTENSION_NAME_SIZE) ==
          0) {
        ++found_extensions_count;
        continue;
      }
    }
  }

  COMET_ASSERT(found_extensions_count >= kExtensionsToCheck_.size(),
               "At least one mandatory extension is missing!");
}
#endif  // COMET_DEBUG
}  // namespace vk
}  // namespace rendering
}  // namespace comet
