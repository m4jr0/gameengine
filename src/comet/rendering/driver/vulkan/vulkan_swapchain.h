// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_SWAPCHAIN_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_SWAPCHAIN_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
struct SwapchainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities{};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

SwapchainSupportDetails QuerySwapchainSupportDetails(VkPhysicalDevice device,
                                                     VkSurfaceKHR surface);
VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& formats);
VkPresentModeKHR ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& present_modes);
VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                            WindowSize current_width,
                            WindowSize current_height);

struct VulkanSwapchainDescr {
  VkInstance instance{VK_NULL_HANDLE};
  VkPhysicalDevice physical_device{VK_NULL_HANDLE};
  VkDevice device{VK_NULL_HANDLE};
  VkSurfaceKHR surface{VK_NULL_HANDLE};
  WindowSize width{0};
  WindowSize height{0};
  bool is_vsync{false};
};

class VulkanSwapchain {
 public:
  VulkanSwapchain() = default;
  VulkanSwapchain(const VulkanSwapchain&) = delete;
  VulkanSwapchain(VulkanSwapchain&&) = delete;
  VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
  VulkanSwapchain& operator=(VulkanSwapchain&&) = delete;
  ~VulkanSwapchain() = default;

  void Set(const VulkanSwapchainDescr& descr);
  void SetSize(WindowSize width, WindowSize height);
  void Initialize();
  void Destroy();
  bool Reload();
  VkResult AcquireNextImage(VkSemaphore semaphore);
  u32 GetCurrentImageIndex() const noexcept;
  VkResult QueuePresent(VkQueue present_queue, VkSemaphore semaphore,
                        u32 image_index);
  bool IsPresentationAvailable() const noexcept;

  bool IsInitialized() const noexcept;
  bool IsReloadNeeded() const noexcept;
  VkFormat GetFormat() const noexcept;
  VkExtent2D GetExtent() const noexcept;
  const std::vector<VkImage>& GetImages() const noexcept;
  const std::vector<VkImageView>& GetImageViews() const noexcept;

 private:
  bool is_initialized_{false};
  bool is_reload_needed_{false};
  VkInstance instance_{VK_NULL_HANDLE};
  VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
  VkDevice device_{VK_NULL_HANDLE};
  WindowSize width_{0};
  WindowSize height_{0};
  u32 current_image_index_{0};
  bool is_vsync_{false};
  VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
  VkSwapchainKHR old_swapchain_{VK_NULL_HANDLE};
  VkSurfaceKHR surface_{VK_NULL_HANDLE};
  VkFormat format_{VK_FORMAT_UNDEFINED};
  VkExtent2D extent_{0, 0};
  std::vector<VkImage> images_{};  // Will be destroyed automatically.
  std::vector<VkImageView> image_views_;

  void CreateImageViews();
  void DestroyImageViews();
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_SWAPCHAIN_H_
