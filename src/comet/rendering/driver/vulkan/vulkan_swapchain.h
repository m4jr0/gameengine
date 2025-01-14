// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_SWAPCHAIN_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_SWAPCHAIN_H_

#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/data/vulkan_frame.h"
#include "comet/rendering/driver/vulkan/data/vulkan_image.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/rendering_common.h"
#include "comet/rendering/window/glfw/vulkan/vulkan_glfw_window.h"

namespace comet {
namespace rendering {
namespace vk {
struct SwapchainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities{};
  Array<VkSurfaceFormatKHR> formats{};
  Array<VkPresentModeKHR> present_modes{};
};

void QuerySwapchainSupportDetails(VkPhysicalDevice physical_device_handle,
                                  VkSurfaceKHR surface_handle,
                                  SwapchainSupportDetails& details);
VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
    const Array<VkSurfaceFormatKHR>& formats);
VkPresentModeKHR ChooseSwapPresentMode(
    const Array<VkPresentModeKHR>& present_modes, bool is_vsync = true,
    bool is_triple_buffering = true);
VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                            WindowSize current_width,
                            WindowSize current_height);

struct SwapchainDescr {
  bool is_vsync{false};
  bool is_triple_buffering{false};
  const VulkanGlfwWindow* window{nullptr};
  Context* context{nullptr};
};

class Swapchain {
 public:
  Swapchain() = delete;
  explicit Swapchain(const SwapchainDescr& descr);
  Swapchain(const Swapchain&) = delete;
  Swapchain(Swapchain&&) = delete;
  Swapchain& operator=(const Swapchain&) = delete;
  Swapchain& operator=(Swapchain&&) = delete;
  ~Swapchain();

  void Initialize();
  void Destroy();

  bool Reload();
  VkResult AcquireNextImage(VkSemaphore semaphore_handle);
  VkResult QueuePresent();
  void HandlePreSwapchainReload();
  void HandlePostSwapchainReload();
  bool IsPresentationAvailable() const noexcept;

  bool IsInitialized() const noexcept;
  VkSwapchainKHR GetHandle() const noexcept;
  operator VkSwapchainKHR() const noexcept;
  bool IsReloadNeeded() const noexcept;
  VkFormat GetFormat() const noexcept;
  const VkExtent2D& GetExtent() const noexcept;
  u32 GetImageCount() const;
  const Array<Image>& GetImages() const noexcept;
  const Image& GetColorImage() const noexcept;
  const Image& GetDepthImage() const noexcept;

 private:
  void InitializeImageViews();
  void InitializeColorResources();
  void InitializeDepthResources();
  void DestroyImageViews();
  void DestroyDepthResources();
  void DestroyColorResources();

  bool is_initialized_{false};
  bool is_reload_needed_{false};
  bool is_vsync_{false};
  bool is_triple_buffering_{false};
  ImageData image_data_{};
  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagRendering};
  VkFormat format_{VK_FORMAT_UNDEFINED};
  VkExtent2D extent_{0, 0};
  Array<Image> images_{};
  Image color_image_{};
  Image depth_image_{};
  VkSwapchainKHR handle_{VK_NULL_HANDLE};
  VkSwapchainKHR old_handle_{VK_NULL_HANDLE};
  const VulkanGlfwWindow* window_{nullptr};
  Context* context_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_SWAPCHAIN_H_
