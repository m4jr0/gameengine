// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DEVICE_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DEVICE_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
struct QueueFamilyIndices {
  std::optional<u32> graphics_family{};
  std::optional<u32> present_family{};
  std::optional<u32> transfer_family{};
};

// Useful for debugging purposes.
constexpr auto kIsSpecificTransferQueue{true};

bool AreQueueFamilyIndicesComplete(const QueueFamilyIndices& indices);
bool IsTransferFamilyInQueueFamilyIndices(const QueueFamilyIndices& indices);
std::vector<u32> GetUniqueIndices(const QueueFamilyIndices& indices);

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physical_device_handle,
                                     VkSurfaceKHR surface_handle);

VkSampleCountFlagBits GetMaxUsableSampleCount(
    VkPhysicalDevice physical_device_handle);

using PhysicalDeviceScore = u32;

struct DeviceDescr {
  VkInstance instance_handle{VK_NULL_HANDLE};
  VkSurfaceKHR surface_handle{VK_NULL_HANDLE};
  bool is_sampler_anisotropy{false};
  bool is_sample_rate_shading{false};
  AntiAliasingType anti_aliasing_type{AntiAliasingType::None};
};

class Device {
 public:
  Device() = delete;
  explicit Device(const DeviceDescr& descr);
  Device(const Device&) = delete;
  Device(Device&&) = delete;
  Device& operator=(const Device&) = delete;
  Device& operator=(Device&&) = delete;
  ~Device();

  void Initialize();
  void Destroy();

  void WaitIdle() const;
  VkFormat ChooseFormat(VkImageTiling tiling, VkFormatFeatureFlags features,
                        const std::vector<VkFormat>& candidates) const;

  VkFormat ChooseDepthFormat() const;

  VkDevice GetHandle() const noexcept;
  operator VkDevice() const noexcept;
  VkPhysicalDevice GetPhysicalDeviceHandle() const noexcept;
  const VkPhysicalDeviceProperties& GetProperties() const noexcept;
  const VkPhysicalDeviceFeatures& GetFeatures() const noexcept;
  const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const noexcept;
  const QueueFamilyIndices& GetQueueFamilyIndices() const noexcept;
  VkQueue GetGraphicsQueueHandle() const noexcept;
  VkQueue GetPresentQueueHandle() const noexcept;
  VkQueue GetTransferQueueHandle() const noexcept;
  VkSampleCountFlagBits GetMsaaSamples() const noexcept;
  bool IsMsaa() const noexcept;
  bool IsInitialized() const noexcept;

 private:
  PhysicalDeviceScore GetPhysicalDeviceScore(
      VkPhysicalDevice physical_device_handle) const;
  void ResolvePhysicalDeviceHandle();
  bool AreDeviceExtensionsAvailable(
      VkPhysicalDevice physical_device_handle) const;

#ifdef COMET_DEBUG
  void CheckRequiredExtensions() const;
#endif  // COMET_DEBUG

  static constexpr std::array<const schar*, 1> kExtensionsToCheck_{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  bool is_initialized_{false};
  bool is_sampler_anisotropy_{false};
  bool is_sample_rate_shading_{false};
  AntiAliasingType anti_aliasing_type_{AntiAliasingType::None};
  VkPhysicalDeviceProperties properties_{};
  VkPhysicalDeviceFeatures features_{};
  VkPhysicalDeviceMemoryProperties memory_properties_{};
  std::vector<VkQueueFamilyProperties> queue_family_properties_{};
  QueueFamilyIndices queue_family_indices_{};
  VkInstance instance_handle_{VK_NULL_HANDLE};
  VkPhysicalDevice physical_device_handle_{VK_NULL_HANDLE};
  VkDevice handle_{VK_NULL_HANDLE};
  VkSurfaceKHR surface_handle_{VK_NULL_HANDLE};
  VkQueue graphics_queue_handle_{
      VK_NULL_HANDLE};  // Will be destroyed automatically.
  VkQueue present_queue_handle_{
      VK_NULL_HANDLE};  // Will be destroyed automatically.
  VkQueue transfer_queue_handle_{
      VK_NULL_HANDLE};  // Will be destroyed automatically.
  static constexpr std::array<const schar*,
#ifdef COMET_VULKAN_DEBUG_MODE
                              2
#else
                              1
#endif  // COMET_VULKAN_DEBUG_MODE
                              >
      kRequiredExtensions_{VK_KHR_SWAPCHAIN_EXTENSION_NAME
#ifdef COMET_VULKAN_DEBUG_MODE
                           ,
                           VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME
#endif  // COMET_VULKAN_DEBUG_MODE
      };

  VkSampleCountFlagBits msaa_samples_{VK_SAMPLE_COUNT_1_BIT};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DEVICE_H_
