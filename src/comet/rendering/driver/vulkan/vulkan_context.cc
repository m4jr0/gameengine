// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_context.h"

#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_alloc.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
#include "comet/core/c_string.h"
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

namespace comet {
namespace rendering {
namespace vk {
Context::Context(const ContextDescr& descr)
    : vulkan_major_version_{descr.vulkan_major_version},
      vulkan_minor_version_{descr.vulkan_minor_version},
      vulkan_patch_version_{descr.vulkan_patch_version},
      vulkan_variant_version_{descr.vulkan_variant_version},
      is_sampler_anisotropy_{descr.is_sampler_anisotropy},
      is_sample_rate_shading_{descr.is_sample_rate_shading},
      max_frames_in_flight_{descr.max_frames_in_flight},
      max_object_count_{descr.max_object_count},
      frame_data_{max_frames_in_flight_},
      instance_handle_{descr.instance_handle},
      device_{descr.device} {
  COMET_ASSERT(instance_handle_ != nullptr,
               "Instance handle provided is null!");
  COMET_ASSERT(device_ != nullptr, "Device provided is null!");
}

Context::~Context() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for context, but it is still initialized!");
}

void Context::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize context, but it is already done!");
  InitializeAllocator();
  InitializeCommands();
  InitializeSyncStructures();
  is_initialized_ = true;
}

void Context::InitializeAllocator() {
  VmaAllocatorCreateInfo create_info{};
  create_info.vulkanApiVersion =
      VK_MAKE_API_VERSION(vulkan_variant_version_, vulkan_major_version_,
                          vulkan_minor_version_, vulkan_patch_version_);
  create_info.physicalDevice = device_->GetPhysicalDeviceHandle();
  create_info.device = *device_;
  create_info.instance = instance_handle_;
  create_info.pAllocationCallbacks =
      MemoryCallbacks::Get().GetAllocCallbacksHandle();
  create_info.pDeviceMemoryCallbacks =
      MemoryCallbacks::Get().GetDeviceCallbacksHandle();
  vmaCreateAllocator(&create_info, &allocator_handle_);
}

void Context::InitializeCommands() {
  auto pool_info{init::GenerateCommandPoolCreateInfo(
      device_->GetQueueFamilyIndices().graphics_family.value(),
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)};

  for (usize i = 0; i < max_frames_in_flight_; ++i) {
    COMET_CHECK_VK(vkCreateCommandPool(*device_, &pool_info, VK_NULL_HANDLE,
                                       &frame_data_[i].command_pool_handle),
                   "Failed to create frame command pool!");

    auto allocate_info{init::GenerateCommandBufferAllocateInfo(
        frame_data_[i].command_pool_handle, 1)};

    COMET_CHECK_VK(
        vkAllocateCommandBuffers(*device_, &allocate_info,
                                 &frame_data_[i].command_buffer_handle),
        "Failed to allocate frame command buffer!");
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
    constexpr auto kDebugLabelLen{31};
    schar debug_label[kDebugLabelLen + 1]{'\0'};
    auto frame_data_len{GetLength("frame_data_")};
    Copy(debug_label, "frame_data_", frame_data_len);
    ConvertToStr(i, debug_label + frame_data_len,
                 kDebugLabelLen - frame_data_len);
    COMET_VK_SET_DEBUG_LABEL(frame_data_[i].command_buffer_handle, debug_label);
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
  }

  pool_info.flags = 0;

  COMET_CHECK_VK(vkCreateCommandPool(*device_, &pool_info, VK_NULL_HANDLE,
                                     &upload_context_.command_pool_handle),
                 "Failed to create upload command pool!");

  if (!IsTransferFamilyInQueueFamilyIndices(device_->GetQueueFamilyIndices())) {
    return;
  }

  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
                    VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  pool_info.queueFamilyIndex =
      device_->GetQueueFamilyIndices().transfer_family.value();

  COMET_CHECK_VK(vkCreateCommandPool(*device_, &pool_info, VK_NULL_HANDLE,
                                     &transfer_command_pool_handle_),
                 "Failed to create transfer command pool!");
}

void Context::InitializeSyncStructures() {
  auto fence_create_info{
      init::GenerateFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT)};
  auto semaphore_create_info{init::GenerateSemaphoreCreateInfo()};

  for (auto& frame_data : frame_data_) {
    COMET_CHECK_VK(vkCreateFence(*device_, &fence_create_info, VK_NULL_HANDLE,
                                 &frame_data.render_fence_handle),
                   "Unable to create frame fence!");

    COMET_CHECK_VK(
        vkCreateSemaphore(*device_, &semaphore_create_info, VK_NULL_HANDLE,
                          &frame_data.present_semaphore_handle),
        "Unable to create frame present semaphore!");

    COMET_CHECK_VK(
        vkCreateSemaphore(*device_, &semaphore_create_info, VK_NULL_HANDLE,
                          &frame_data.render_semaphore_handle),
        "Unable to create frame render semaphore!");
  }

  fence_create_info.flags = 0;

  COMET_CHECK_VK(vkCreateFence(*device_, &fence_create_info, VK_NULL_HANDLE,
                               &upload_context_.upload_fence_handle),
                 "Unable to create upload fence!");
}

void Context::BindImageData(const ImageData* image_data) {
  image_data_ = image_data;
  COMET_ASSERT(image_data_ != nullptr, "Image data bound is null!");
}

void Context::UnbindImageData() { image_data_ = nullptr; }

void Context::Destroy() {
  COMET_ASSERT(is_initialized_,
               "Tried to destroy context, but it is not initialized!");
  DestroyAllocator();
  DestroyCommands();
  DestroySyncStructures();

  vulkan_major_version_ = 0;
  vulkan_minor_version_ = 0;
  vulkan_patch_version_ = 0;
  vulkan_variant_version_ = 0;
  is_sampler_anisotropy_ = false;
  is_sample_rate_shading_ = false;
  frame_count_ = 0;
  frame_in_flight_index_ = 0;
  max_frames_in_flight_ = 2;
  max_object_count_ = 0;
  upload_context_ = {};
  frame_data_.clear();
  device_ = nullptr;
  image_data_ = nullptr;
  instance_handle_ = VK_NULL_HANDLE;
  transfer_command_pool_handle_ = VK_NULL_HANDLE;

  is_initialized_ = false;
}

void Context::DestroyAllocator() {
  if (allocator_handle_ == VK_NULL_HANDLE) {
    return;
  }

  vmaDestroyAllocator(allocator_handle_);
  allocator_handle_ = VK_NULL_HANDLE;
}

void Context::DestroyCommands() {
  for (auto& frame_data : frame_data_) {
    if (frame_data.command_pool_handle != VK_NULL_HANDLE) {
      vkDestroyCommandPool(*device_, frame_data.command_pool_handle,
                           VK_NULL_HANDLE);
      frame_data.command_pool_handle = VK_NULL_HANDLE;
    }
  }

  if (transfer_command_pool_handle_ != VK_NULL_HANDLE) {
    vkDestroyCommandPool(*device_, transfer_command_pool_handle_,
                         VK_NULL_HANDLE);
    transfer_command_pool_handle_ = VK_NULL_HANDLE;
  }

  if (upload_context_.command_pool_handle != VK_NULL_HANDLE) {
    vkDestroyCommandPool(*device_, upload_context_.command_pool_handle,
                         VK_NULL_HANDLE);
    upload_context_.command_pool_handle = VK_NULL_HANDLE;
  }
}

void Context::DestroySyncStructures() {
  for (auto& frame_data : frame_data_) {
    if (frame_data.render_fence_handle != VK_NULL_HANDLE) {
      vkDestroyFence(*device_, frame_data.render_fence_handle, VK_NULL_HANDLE);
      frame_data.render_fence_handle = VK_NULL_HANDLE;
    }

    if (frame_data.present_semaphore_handle != VK_NULL_HANDLE) {
      vkDestroySemaphore(*device_, frame_data.present_semaphore_handle,
                         VK_NULL_HANDLE);
      frame_data.present_semaphore_handle = VK_NULL_HANDLE;
    }

    if (frame_data.render_semaphore_handle != VK_NULL_HANDLE) {
      vkDestroySemaphore(*device_, frame_data.render_semaphore_handle,
                         VK_NULL_HANDLE);
      frame_data.render_semaphore_handle = VK_NULL_HANDLE;
    }
  }

  if (upload_context_.upload_fence_handle != VK_NULL_HANDLE) {
    vkDestroyFence(*device_, upload_context_.upload_fence_handle,
                   VK_NULL_HANDLE);
    upload_context_.upload_fence_handle = VK_NULL_HANDLE;
  }
}

void Context::HandlePreSwapchainReload() { DestroyCommands(); }

void Context::HandlePostSwapchainReload() {
  InitializeCommands();
  frame_in_flight_index_ = 0;
}

void Context::GoToNextFrame() noexcept {
  ++frame_in_flight_index_ %= max_frames_in_flight_;
  ++frame_count_;
}

FrameData& Context::GetFrameData(FrameInFlightIndex frame) {
  if (frame == kInvalidFrameInFlightIndex) {
    frame = frame_in_flight_index_;
  }

  COMET_ASSERT(frame < frame_data_.size(),
               "Requesting command generation from frame #", frame,
               ", but frame count is ", frame_data_.size(), "!");

  return frame_data_[frame];
}

const FrameData& Context::GetFrameData(FrameInFlightIndex frame) const {
  if (frame == kInvalidFrameInFlightIndex) {
    frame = frame_in_flight_index_;
  }

  COMET_ASSERT(frame < frame_data_.size(),
               "Requesting command generation from frame #", frame,
               ", but frame count is ", frame_data_.size(), "!");

  return frame_data_[frame];
}

bool Context::IsSamplerAnisotropy() const noexcept {
  return is_sampler_anisotropy_;
}

bool Context::IsSampleRateShading() const noexcept {
  return is_sample_rate_shading_;
}

ImageIndex Context::GetImageIndex() const {
  COMET_ASSERT(image_data_ != nullptr,
               "Image index was required, but is image data null!");
  return image_data_->image_index;
}

ImageIndex Context::GetImageCount() const {
  COMET_ASSERT(image_data_ != nullptr,
               "Image count was required, but image data is null!");
  return image_data_->image_count;
}

FrameIndex Context::GetFrameCount() const noexcept { return frame_count_; }

FrameInFlightIndex Context::GetFrameInFlightIndex() const noexcept {
  return frame_in_flight_index_;
}

FrameInFlightIndex Context::GetMaxFramesInFlight() const noexcept {
  return max_frames_in_flight_;
}

VkInstance Context::GetInstanceHandle() const noexcept {
  return instance_handle_;
}

const Device& Context::GetDevice() const noexcept { return *device_; }

UploadContext& Context::GetUploadContext() noexcept { return upload_context_; }

const UploadContext& Context::GetUploadContext() const noexcept {
  return upload_context_;
}

VkPhysicalDevice Context::GetPhysicalDeviceHandle() const noexcept {
  return device_->GetPhysicalDeviceHandle();
}

u8 Context::GetVulkanMajorVersion() const noexcept {
  return vulkan_major_version_;
}

u8 Context::GetVulkanMinorVersion() const noexcept {
  return vulkan_minor_version_;
}

u8 Context::GetVulkanPatchVersion() const noexcept {
  return vulkan_patch_version_;
}

u8 Context::GetVulkanVariantVersion() const noexcept {
  return vulkan_variant_version_;
}

usize Context::GetMaxObjectCount() const noexcept { return max_object_count_; }

VmaAllocator Context::GetAllocatorHandle() const noexcept {
  return allocator_handle_;
}

VkCommandPool Context::GetTransferCommandPoolHandle() const {
  if (transfer_command_pool_handle_ == VK_NULL_HANDLE) {
    return GetFrameData().command_pool_handle;
  }

  return transfer_command_pool_handle_;
}

bool Context::IsInitialized() const noexcept { return is_initialized_; }
}  // namespace vk
}  // namespace rendering
}  // namespace comet