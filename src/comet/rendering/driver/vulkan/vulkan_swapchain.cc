// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_swapchain.h"

#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/driver/vulkan/vulkan_image.h"
#include "comet/rendering/driver/vulkan/vulkan_initializers.h"

namespace comet {
namespace rendering {
namespace vk {
SwapchainSupportDetails QuerySwapchainSupportDetails(VkPhysicalDevice device,
                                                     VkSurfaceKHR surface) {
  SwapchainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  u32 format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                       VK_NULL_HANDLE);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                         details.formats.data());
  }

  u32 present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &present_mode_count, VK_NULL_HANDLE);

  COMET_ASSERT(present_mode_count > 0, "No present mode available!!");

  details.present_modes.resize(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &present_mode_count, details.present_modes.data());

  return details;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& formats) {
  for (const auto& available_format : formats) {
    // TODO(m4jr0): Retrieve the "best" settings from a configuration file.
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_format;
    }
  }

  // TODO(m4jr0): Rank the other available formats and return the best one.
  return formats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& present_modes) {
  for (const auto& available_present_mode : present_modes) {
    // TODO(m4jr0): Retrieve the "sync" mode from the settings.
    if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_present_mode;
    }
  }

  // Return present mode guaranteed to be available.
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                            WindowSize current_width,
                            WindowSize current_height) {
  if (capabilities.currentExtent.width != static_cast<u32>(-1)) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D extent{current_width, current_height};

    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);

    extent.height =
        std::clamp(extent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return extent;
  }
}

void VulkanSwapchain::Set(const VulkanSwapchainDescr& descr) {
  instance_ = descr.instance;
  physical_device_ = descr.physical_device;
  device_ = descr.device;
  surface_ = descr.surface;
  width_ = descr.width;
  height_ = descr.height;
  is_vsync_ = descr.is_vsync;
}

void VulkanSwapchain::SetSize(WindowSize width, WindowSize height) {
  width_ = width;
  height_ = height;
}

void VulkanSwapchain::Initialize() {
  COMET_ASSERT(!is_initialized_, "Swapchain is already initialized!");
  ;
  const auto details{QuerySwapchainSupportDetails(physical_device_, surface_)};
  extent_ = ChooseSwapExtent(details.capabilities, width_, height_);

  if (extent_.width == 0 || extent_.height == 0) {
    // The extent is flat, nothing to do.
    is_reload_needed_ = true;
    return;
  }

  auto surface_format{ChooseSwapSurfaceFormat(details.formats)};
  auto present_mode{ChooseSwapPresentMode(details.present_modes)};
  format_ = surface_format.format;

  auto image_count{details.capabilities.minImageCount + 1};

  if (details.capabilities.maxImageCount > 0 &&
      image_count > details.capabilities.maxImageCount) {
    image_count = details.capabilities.maxImageCount;
  }

  const auto queue_family_indices{
      FindQueueFamilies(physical_device_, surface_)};

  auto queue_family_unique_indices{queue_family_indices.GetUniqueIndices()};

  auto create_info{init::GetSwapchainCreateInfo(
      surface_, surface_format, extent_, present_mode, details,
      queue_family_unique_indices, image_count)};

  create_info.oldSwapchain = old_swapchain_;

  COMET_CHECK_VK(
      vkCreateSwapchainKHR(device_, &create_info, VK_NULL_HANDLE, &swapchain_),
      "Failed to create swap chain");

  vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, VK_NULL_HANDLE);
  images_.resize(image_count);
  vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, images_.data());
  CreateImageViews();

  is_initialized_ = true;
}

void VulkanSwapchain::Destroy() {
  DestroyImageViews();

  if (swapchain_ != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device_, swapchain_, VK_NULL_HANDLE);
    swapchain_ = VK_NULL_HANDLE;
  }

  is_initialized_ = false;
}

bool VulkanSwapchain::Reload() {
  // Copy data needed the first time.
  if (!is_reload_needed_) {
    is_initialized_ = false;
    old_swapchain_ = swapchain_;
    DestroyImageViews();
  }

  is_reload_needed_ = true;

  // Reload has been requested, but no draw area is available at this moment.
  if (!IsPresentationAvailable()) {
    return false;
  }

  Initialize();

  if (!is_initialized_) {
    return false;
  }

  old_swapchain_ = VK_NULL_HANDLE;
  is_reload_needed_ = false;
  return true;
}

VkResult VulkanSwapchain::AcquireNextImage(VkSemaphore semaphore) {
  return vkAcquireNextImageKHR(device_, swapchain_, static_cast<u64>(-1),
                               semaphore, VK_NULL_HANDLE,
                               &current_image_index_);
}

u32 VulkanSwapchain::GetCurrentImageIndex() const noexcept {
  return current_image_index_;
}

VkResult VulkanSwapchain::QueuePresent(VkQueue present_queue,
                                       VkSemaphore semaphore, u32 image_index) {
  std::array<VkSwapchainKHR, 1> swapchains{{swapchain_}};

  auto present_info{init::GetPresentInfo()};

  if (semaphore != VK_NULL_HANDLE) {
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &semaphore;
  }

  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain_;
  present_info.pImageIndices = &image_index;
  present_info.pResults = VK_NULL_HANDLE;

  return vkQueuePresentKHR(present_queue, &present_info);
}

bool VulkanSwapchain::IsPresentationAvailable() const noexcept {
  return width_ > 0 && height_ > 0;
}

bool VulkanSwapchain::IsInitialized() const noexcept { return is_initialized_; }

bool VulkanSwapchain::IsReloadNeeded() const noexcept {
  return is_reload_needed_;
}

VkFormat VulkanSwapchain::GetFormat() const noexcept { return format_; }

VkExtent2D VulkanSwapchain::GetExtent() const noexcept { return extent_; }

const std::vector<VkImage>& VulkanSwapchain::GetImages() const noexcept {
  return images_;
}

const std::vector<VkImageView>& VulkanSwapchain::GetImageViews()
    const noexcept {
  return image_views_;
}

void VulkanSwapchain::CreateImageViews() {
  image_views_.resize(images_.size());

  for (uindex i{0}; i < images_.size(); ++i) {
    image_views_[i] = CreateImageView(device_, images_[i], format_,
                                      VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }
}

void VulkanSwapchain::DestroyImageViews() {
  for (auto& image_view : image_views_) {
    if (image_view == VK_NULL_HANDLE) {
      continue;
    }

    vkDestroyImageView(device_, image_view, VK_NULL_HANDLE);
    image_view = VK_NULL_HANDLE;
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
