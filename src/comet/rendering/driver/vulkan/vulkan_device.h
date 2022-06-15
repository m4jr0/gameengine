// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DEVICE_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DEVICE_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

namespace comet {
namespace rendering {
namespace vk {
// Useful for debugging purposes.
constexpr auto kIsSpecificTransferQueue{true};

struct QueueFamilyIndices {
  std::optional<u32> graphics_family;
  std::optional<u32> present_family;
  std::optional<u32> transfer_family;

  bool IsComplete() const;
  bool IsSpecificTransferFamily() const;
  std::vector<u32> GetUniqueIndices() const;
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device,
                                     VkSurfaceKHR surface);

VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice physical_device);

using PhysicalDeviceScore = u32;

PhysicalDeviceScore GetPhysicalDeviceScore(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface,
    const std::vector<const char*>& required_extensions);

VkPhysicalDevice GetBestPhysicalDevice(
    VkInstance instance, VkSurfaceKHR surface,
    const std::vector<const char*>& required_extensions);

bool AreDeviceExtensionsAvailable(VkPhysicalDevice physical_device,
                                  const std::vector<const char*>& extensions);

struct VulkanDeviceDescr {
  VkPhysicalDevice physical_device{VK_NULL_HANDLE};
  VkSurfaceKHR surface{VK_NULL_HANDLE};
  bool is_sampler_anisotropy{false};
  bool is_sample_rate_shading{false};
};

class VulkanDevice {
 public:
  explicit VulkanDevice() = default;
  VulkanDevice(const VulkanDevice&) = delete;
  VulkanDevice(VulkanDevice&&) = delete;
  VulkanDevice& operator=(const VulkanDevice&) = delete;
  VulkanDevice& operator=(VulkanDevice&&) = delete;
  ~VulkanDevice() = default;

  void Initialize(const VulkanDeviceDescr& descr);
  void Destroy();

  template <typename... ExtensionNames>
  void SetRequiredExtensions(ExtensionNames&&... extension_names) {
    required_extensions_ = {extension_names...};
  }

  template <typename ExtensionNames>
  void SetRequiredExtensions(ExtensionNames&& extension_names) {
    required_extensions_ = std::forward<ExtensionNames>(extension_names);
  }

  VkFormat ChooseFormat(VkImageTiling tiling, VkFormatFeatureFlags features,
                        const std::vector<VkFormat>& candidates);

  VkFormat ChooseDepthFormat();

  operator VkDevice() const noexcept;
  VkDevice GetDevice() const noexcept;
  VkPhysicalDevice GetPhysicalDevice() const noexcept;
  const VkPhysicalDeviceProperties& GetProperties() const noexcept;
  const VkPhysicalDeviceFeatures& GetFeatures() const noexcept;
  const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const noexcept;
  const QueueFamilyIndices& GetQueueFamilyIndices() const noexcept;
  VkQueue GetGraphicsQueue() const noexcept;
  VkQueue GetPresentQueue() const noexcept;
  VkQueue GetTransferQueue() const noexcept;
  VkSampleCountFlagBits GetMsaaSamples() const noexcept;

 private:
  // Handles.
  VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
  VkDevice device_{VK_NULL_HANDLE};

  // Properties and features.
  VkPhysicalDeviceProperties properties_{};
  VkPhysicalDeviceFeatures features_{};
  VkPhysicalDeviceMemoryProperties memory_properties_{};

  // Queues.
  std::vector<VkQueueFamilyProperties> queue_family_properties_;
  QueueFamilyIndices queue_family_indices_{};
  VkQueue graphics_queue_{VK_NULL_HANDLE};  // Will be destroyed automatically.
  VkQueue present_queue_{VK_NULL_HANDLE};   // Will be destroyed automatically.
  VkQueue transfer_queue_{VK_NULL_HANDLE};  // Will be destroyed automatically.

  // Misc.
  static constexpr std::array<const char*, 1> kExtensionsToCheck_{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  std::vector<const char*> required_extensions_;
  VkSampleCountFlagBits msaa_samples_{VK_SAMPLE_COUNT_1_BIT};

#ifdef COMET_DEBUG
  // Debug.
  void CheckRequiredExtensions();
#endif  // COMET_DEBUG
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DEVICE_H_
