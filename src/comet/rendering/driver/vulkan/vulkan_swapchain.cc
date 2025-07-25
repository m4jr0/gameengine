// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_swapchain.h"

#include "comet/core/frame/frame_utils.h"
#include "comet/math/math_common.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_image_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_alloc.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/driver/vulkan/vulkan_device.h"

namespace comet {
namespace rendering {
namespace vk {
void QuerySwapchainSupportDetails(VkPhysicalDevice physical_device_handle,
                                  VkSurfaceKHR surface_handle,
                                  SwapchainSupportDetails& details) {
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physical_device_handle, surface_handle, &details.capabilities);

  u32 format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_handle, surface_handle,
                                       &format_count, VK_NULL_HANDLE);

  if (format_count != 0) {
    details.formats.Resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_handle, surface_handle,
                                         &format_count,
                                         details.formats.GetData());
  }

  u32 present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_handle,
                                            surface_handle, &present_mode_count,
                                            VK_NULL_HANDLE);

  COMET_ASSERT(present_mode_count > 0, "No present mode available!!");

  details.present_modes.Resize(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_handle,
                                            surface_handle, &present_mode_count,
                                            details.present_modes.GetData());
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
    const Array<VkSurfaceFormatKHR>& formats) {
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
    const Array<VkPresentModeKHR>& present_modes, bool is_vsync,
    bool is_triple_buffering) {
  if (is_vsync) {
    if (!is_triple_buffering) {
      return VK_PRESENT_MODE_FIFO_KHR;
    }

    for (const auto& present_mode : present_modes) {
      if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return present_mode;
      }
    }
  } else {
    for (const auto& present_mode : present_modes) {
      if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        return present_mode;
      }
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

    extent.width = math::Clamp(extent.width, capabilities.minImageExtent.width,
                               capabilities.maxImageExtent.width);

    extent.height =
        math::Clamp(extent.height, capabilities.minImageExtent.height,
                    capabilities.maxImageExtent.height);

    return extent;
  }
}

Swapchain::Swapchain(const SwapchainDescr& descr)
    : is_vsync_{descr.is_vsync},
      is_triple_buffering_{descr.is_triple_buffering},
      window_{descr.window},
      context_{descr.context} {
  COMET_ASSERT(window_ != nullptr, "Window provided is null!");
  COMET_ASSERT(context_ != nullptr, "Context provided is null!");
}

Swapchain::~Swapchain() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for swapchain, but it is still initialized!");
}

void Swapchain::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize Swapchain, but it is already done!");
  images_ = Array<Image>{&allocator_};

  const auto& device{context_->GetDevice()};
  auto physical_device_handle{device.GetPhysicalDeviceHandle()};

  SwapchainSupportDetails details{};
  details.formats = Array<VkSurfaceFormatKHR>{&allocator_};
  details.present_modes = Array<VkPresentModeKHR>{&allocator_};
  QuerySwapchainSupportDetails(physical_device_handle, *window_, details);

  extent_ = ChooseSwapExtent(details.capabilities, window_->GetWidth(),
                             window_->GetHeight());

  if (extent_.width == 0 || extent_.height == 0) {
    // The extent is flat, nothing to do.
    is_reload_needed_ = true;
    return;
  }

  auto surface_format{ChooseSwapSurfaceFormat(details.formats)};
  auto present_mode{ChooseSwapPresentMode(details.present_modes, is_vsync_,
                                          is_triple_buffering_)};
  format_ = surface_format.format;

  auto image_count{details.capabilities.minImageCount + 1};

  if (details.capabilities.maxImageCount > 0 &&
      image_count > details.capabilities.maxImageCount) {
    image_count = details.capabilities.maxImageCount;
  }

  const auto queue_family_indices{
      FindQueueFamilies(physical_device_handle, *window_)};

  auto queue_family_unique_indices{GetUniqueIndices(queue_family_indices)};

  auto create_info{init::GenerateSwapchainCreateInfo(
      *window_, surface_format, extent_, present_mode, details,
      queue_family_unique_indices, image_count)};

  create_info.oldSwapchain = old_handle_;

  COMET_CHECK_VK(
      vkCreateSwapchainKHR(device, &create_info,
                           MemoryCallbacks::Get().GetAllocCallbacksHandle(),
                           &handle_),
      "Failed to create swap chain");

  if (old_handle_ != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device, old_handle_,
                          MemoryCallbacks::Get().GetAllocCallbacksHandle());
    old_handle_ = VK_NULL_HANDLE;
  }

  vkGetSwapchainImagesKHR(device, handle_, &image_count, VK_NULL_HANDLE);
  images_.Reserve(image_count);
  frame::FrameArray<VkImage> image_handles{};
  image_handles.Resize(image_count);
  vkGetSwapchainImagesKHR(device, handle_, &image_count,
                          image_handles.GetData());

  for (auto image_handle : image_handles) {
    Image image{};
    image.handle = image_handle;
    images_.PushBack(image);
  }

  image_data_ = {0, static_cast<ImageIndex>(image_handles.GetSize())};
  context_->BindImageData(&image_data_);

  InitializeImageViews();
  InitializeRenderSemaphores();

  if (!is_reload_needed_) {
    InitializeColorResources();
    InitializeDepthResources();
  }

  is_initialized_ = true;
}

void Swapchain::Destroy() {
  COMET_ASSERT(is_initialized_,
               "Tried to destroy Swapchain, but it is not initialized!");
  context_->UnbindImageData();
  DestroyRenderSemaphores();
  DestroyImageViews();
  DestroyDepthResources();
  DestroyColorResources();
  auto& device{context_->GetDevice()};

  if (old_handle_ != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device, old_handle_,
                          MemoryCallbacks::Get().GetAllocCallbacksHandle());
    old_handle_ = VK_NULL_HANDLE;
  }

  if (handle_ != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device, handle_,
                          MemoryCallbacks::Get().GetAllocCallbacksHandle());
    handle_ = VK_NULL_HANDLE;
  }

  is_reload_needed_ = false;
  is_vsync_ = false;
  image_data_ = {};
  format_ = VK_FORMAT_UNDEFINED;
  extent_ = {0, 0};
  window_ = nullptr;
  context_ = nullptr;
  is_initialized_ = false;
}

bool Swapchain::Reload() {
  // Copy data needed the first time.
  if (!is_reload_needed_) {
    is_initialized_ = false;
    old_handle_ = handle_;
    DestroyImageViews();
    DestroyRenderSemaphores();
  }

  // Reload has been requested, but no draw area is available at this moment.
  if (!is_reload_needed_ && !IsPresentationAvailable()) {
    is_reload_needed_ = true;
    return false;
  }

  Initialize();

  if (!is_initialized_) {
    return false;
  }

  old_handle_ = VK_NULL_HANDLE;
  is_reload_needed_ = false;
  return true;
}

VkResult Swapchain::AcquireNextImage(VkSemaphore semaphore_handle) {
  auto result{vkAcquireNextImageKHR(context_->GetDevice(), handle_,
                                    static_cast<u64>(-1), semaphore_handle,
                                    VK_NULL_HANDLE, &image_data_.image_index)};

  if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
    image_data_.render_semaphore_handle =
        render_semaphore_handles_[image_data_.image_index %
                                  render_semaphore_handles_.GetSize()];
  } else {
    image_data_.render_semaphore_handle = VK_NULL_HANDLE;
  }

  return result;
}

VkResult Swapchain::QueuePresent() {
  auto present_queue_handle{context_->GetDevice().GetPresentQueueHandle()};
  auto semaphore_handle{image_data_.render_semaphore_handle};
  auto present_info{init::GeneratePresentInfo()};

  if (semaphore_handle != VK_NULL_HANDLE) {
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &semaphore_handle;
  }

  present_info.swapchainCount = 1;
  present_info.pSwapchains = &handle_;
  present_info.pImageIndices = &image_data_.image_index;
  present_info.pResults = VK_NULL_HANDLE;

  return vkQueuePresentKHR(present_queue_handle, &present_info);
}

void Swapchain::HandlePreSwapchainReload() {
  DestroyDepthResources();
  DestroyColorResources();
}

void Swapchain::HandlePostSwapchainReload() {
  InitializeDepthResources();
  InitializeColorResources();
}

bool Swapchain::IsPresentationAvailable() const noexcept {
  return !window_->IsFlat() && is_initialized_ && !is_reload_needed_ &&
         extent_.width > 0 && extent_.height > 0;
}

bool Swapchain::IsInitialized() const noexcept { return is_initialized_; }

VkSwapchainKHR Swapchain::GetHandle() const noexcept { return handle_; }

Swapchain::operator VkSwapchainKHR() const noexcept { return GetHandle(); }

bool Swapchain::IsReloadNeeded() const noexcept { return is_reload_needed_; }

VkFormat Swapchain::GetFormat() const noexcept { return format_; }

const VkExtent2D& Swapchain::GetExtent() const noexcept { return extent_; }

u32 Swapchain::GetImageCount() const {
  return static_cast<u32>(images_.GetSize());
}

const Array<Image>& Swapchain::GetImages() const noexcept { return images_; }

const Image& Swapchain::GetColorImage() const noexcept { return color_image_; }

const Image& Swapchain::GetDepthImage() const noexcept { return depth_image_; }

void Swapchain::InitializeImageViews() {
  const auto& device{context_->GetDevice()};

  for (auto& image : images_) {
    image.image_view_handle = GenerateImageView(device, image.handle, format_,
                                                VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }
}

void Swapchain::InitializeRenderSemaphores() {
  render_semaphore_handles_ = Array<VkSemaphore>{&allocator_};
  auto render_semaphore_count{images_.GetSize()};
  render_semaphore_handles_.Resize(render_semaphore_count);
  auto device_handle{static_cast<VkDevice>(context_->GetDevice())};
  auto semaphore_create_info{init::GenerateSemaphoreCreateInfo()};

  for (usize i{0}; i < render_semaphore_count; ++i) {
    COMET_CHECK_VK(
        vkCreateSemaphore(device_handle, &semaphore_create_info, VK_NULL_HANDLE,
                          &render_semaphore_handles_[i]),
        "Unable to create frame render semaphore!");
  }
}

void Swapchain::InitializeColorResources() {
  auto& device{context_->GetDevice()};

  if (!device.IsMsaa()) {
    return;
  }

  color_image_.allocator_handle = context_->GetAllocatorHandle();

  GenerateImage(color_image_, device, extent_.width, extent_.height, 1,
                device.GetMsaaSamples(), format_, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // One 1 mip level, since it is  enforces the usage of only one when
  // there are more than one sample per pixel (and we don't need mimaps
  // anyway).
  color_image_.image_view_handle = GenerateImageView(
      device, color_image_.handle, format_, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void Swapchain::InitializeDepthResources() {
  auto& device{context_->GetDevice()};
  auto depth_format{device.ChooseDepthFormat()};
  depth_image_.allocator_handle = context_->GetAllocatorHandle();

  GenerateImage(depth_image_, device, extent_.width, extent_.height, 1,
                device.GetMsaaSamples(), depth_format, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  depth_image_.image_view_handle = GenerateImageView(
      device, depth_image_.handle, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void Swapchain::DestroyRenderSemaphores() {
  auto device_handle{static_cast<VkDevice>(context_->GetDevice())};

  for (auto handle : render_semaphore_handles_) {
    if (handle != VK_NULL_HANDLE) {
      vkDestroySemaphore(device_handle, handle, VK_NULL_HANDLE);
    }
  }

  render_semaphore_handles_.Destroy();
}

void Swapchain::DestroyImageViews() {
  for (auto& image : images_) {
    if (image.image_view_handle == VK_NULL_HANDLE) {
      continue;
    }

    vkDestroyImageView(context_->GetDevice(), image.image_view_handle,
                       MemoryCallbacks::Get().GetAllocCallbacksHandle());
    image.image_view_handle = VK_NULL_HANDLE;
  }

  images_.Destroy();
}

void Swapchain::DestroyDepthResources() {
  if (depth_image_.image_view_handle != VK_NULL_HANDLE) {
    vkDestroyImageView(context_->GetDevice(), depth_image_.image_view_handle,
                       MemoryCallbacks::Get().GetAllocCallbacksHandle());
    depth_image_.image_view_handle = VK_NULL_HANDLE;
  }

  if (IsImageInitialized(depth_image_)) {
    DestroyImage(depth_image_);
  }
}

void Swapchain::DestroyColorResources() {
  if (color_image_.image_view_handle != VK_NULL_HANDLE) {
    vkDestroyImageView(context_->GetDevice(), color_image_.image_view_handle,
                       MemoryCallbacks::Get().GetAllocCallbacksHandle());
    color_image_.image_view_handle = VK_NULL_HANDLE;
  }

  if (IsImageInitialized(color_image_)) {
    DestroyImage(color_image_);
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
