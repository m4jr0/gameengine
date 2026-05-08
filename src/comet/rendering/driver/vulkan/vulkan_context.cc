// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_context.h"
////////////////////////////////////////////////////////////////////////////////

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
      instance_handle_{descr.instance_handle},
      device_{descr.device} {
  COMET_ASSERT(instance_handle_ != VK_NULL_HANDLE, "Context::Context",
               "instance handle is null");
  COMET_ASSERT(device_ != nullptr, "Context::Context", "device is null");
}

Context::~Context() {
  COMET_ASSERT(!is_initialized_, "Context::~Context",
               "context is still initialized");
}

void Context::Initialize() {
  COMET_ASSERT(!is_initialized_, "Context::Initialize",
               "context is already initialized");

  InitializeAllocator();
  InitializeFrameData();
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

void Context::InitializeFrameData() {
  frame_data_ = Array<FrameData>{&allocator_};
  frame_data_.Resize(max_frames_in_flight_);
}

void Context::InitializeCommands() {
  auto pool_info{init::GenerateCommandPoolCreateInfo(
      device_->GetGraphicsQueueContext().family_index,
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)};

  for (usize i{0}; i < max_frames_in_flight_; ++i) {
    COMET_CHECK_VK(vkCreateCommandPool(*device_, &pool_info, VK_NULL_HANDLE,
                                       &frame_data_[i].command_pool_handle),
                   "Context::InitializeCommands",
                   "frame command pool creation failed", "frame", i);

    const auto allocate_info{init::GenerateCommandBufferAllocateInfo(
        frame_data_[i].command_pool_handle, 1)};

    COMET_CHECK_VK(
        vkAllocateCommandBuffers(*device_, &allocate_info,
                                 &frame_data_[i].command_buffer_handle),
        "Context::InitializeCommands", "frame command buffer allocation failed",
        "frame", i);
  }

  if (!device_->IsUploadQueueGraphics()) {
    auto upload_pool_info{init::GenerateCommandPoolCreateInfo(
        device_->GetUploadQueueContext().family_index,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)};

    COMET_CHECK_VK(
        vkCreateCommandPool(*device_, &upload_pool_info, VK_NULL_HANDLE,
                            &upload_command_pool_handle_),
        "Context::InitializeCommands", "upload command pool creation failed");
  }

  for (usize i{0}; i < max_frames_in_flight_; ++i) {
    const auto upload_allocate_info{init::GenerateCommandBufferAllocateInfo(
        device_->IsUploadQueueGraphics() ? frame_data_[i].command_pool_handle
                                         : upload_command_pool_handle_,
        1)};

    COMET_CHECK_VK(
        vkAllocateCommandBuffers(*device_, &upload_allocate_info,
                                 &frame_data_[i].upload_command_buffer_handle),
        "Context::InitializeCommands",
        "upload command buffer allocation failed", "frame", i);
  }
}

void Context::InitializeSyncStructures() {
  const auto fence_create_info{
      init::GenerateFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT)};
  auto semaphore_create_info{init::GenerateSemaphoreCreateInfo()};

  for (auto& frame_data : frame_data_) {
    COMET_CHECK_VK(vkCreateFence(*device_, &fence_create_info, VK_NULL_HANDLE,
                                 &frame_data.render_fence_handle),
                   "Context::InitializeSyncStructures",
                   "frame fence creation failed");

    COMET_CHECK_VK(
        vkCreateSemaphore(*device_, &semaphore_create_info, VK_NULL_HANDLE,
                          &frame_data.present_semaphore_handle),
        "Context::InitializeSyncStructures",
        "frame present semaphore creation failed");

    COMET_CHECK_VK(vkCreateFence(*device_, &fence_create_info, VK_NULL_HANDLE,
                                 &frame_data.upload_fence_handle),
                   "Context::InitializeSyncStructures",
                   "upload fence creation failed");
  }

  VkSemaphoreTypeCreateInfo upload_semaphore_type_info{};
  upload_semaphore_type_info.sType =
      VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
  upload_semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
  upload_semaphore_type_info.initialValue = 0;

  semaphore_create_info.pNext = &upload_semaphore_type_info;

  COMET_CHECK_VK(vkCreateSemaphore(*device_, &semaphore_create_info,
                                   VK_NULL_HANDLE, &upload_semaphore_handle_),
                 "Context::InitializeSyncStructures",
                 "upload semaphore creation failed");

  upload_timeline_value_ = 0;
}

void Context::BindImageData(const ImageData* image_data) {
  image_data_ = image_data;
  COMET_ASSERT(image_data_ != nullptr, "Context::BindImageData",
               "image data is null");
}

void Context::UnbindImageData() { image_data_ = nullptr; }

void Context::Destroy() {
  COMET_ASSERT(is_initialized_, "Context::Destroy",
               "context is not initialized");

  DestroySyncStructures();
  DestroyCommands();
  DestroyFrameData();
  DestroyAllocator();

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
  upload_timeline_value_ = 0;

  instance_handle_ = VK_NULL_HANDLE;

  allocator_handle_ = VK_NULL_HANDLE;
  upload_command_pool_handle_ = VK_NULL_HANDLE;
  upload_semaphore_handle_ = VK_NULL_HANDLE;

  device_ = nullptr;
  image_data_ = nullptr;

  is_initialized_ = false;
}

void Context::DestroyAllocator() {
  if (allocator_handle_ == VK_NULL_HANDLE) {
    return;
  }

  vmaDestroyAllocator(allocator_handle_);
  allocator_handle_ = VK_NULL_HANDLE;
}

void Context::DestroyFrameData() { frame_data_.Release(); }

void Context::DestroyCommands() {
  for (auto& frame_data : frame_data_) {
    frame_data.command_buffer_handle = VK_NULL_HANDLE;
    frame_data.upload_command_buffer_handle = VK_NULL_HANDLE;
    frame_data.upload_timeline_wait_value = 0;
    frame_data.requires_upload_ownership_acquire = false;
    frame_data.has_upload_submission = false;

    if (frame_data.command_pool_handle != VK_NULL_HANDLE) {
      vkDestroyCommandPool(*device_, frame_data.command_pool_handle,
                           VK_NULL_HANDLE);
      frame_data.command_pool_handle = VK_NULL_HANDLE;
    }
  }

  if (upload_command_pool_handle_ != VK_NULL_HANDLE) {
    vkDestroyCommandPool(*device_, upload_command_pool_handle_, VK_NULL_HANDLE);
    upload_command_pool_handle_ = VK_NULL_HANDLE;
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

    if (frame_data.upload_fence_handle != VK_NULL_HANDLE) {
      vkDestroyFence(*device_, frame_data.upload_fence_handle, VK_NULL_HANDLE);
      frame_data.upload_fence_handle = VK_NULL_HANDLE;
    }
  }

  if (upload_semaphore_handle_ != VK_NULL_HANDLE) {
    vkDestroySemaphore(*device_, upload_semaphore_handle_, VK_NULL_HANDLE);
    upload_semaphore_handle_ = VK_NULL_HANDLE;
  }
}

void Context::HandlePreSwapchainReload() { DestroyCommands(); }

void Context::HandlePostSwapchainReload() {
  InitializeCommands();
  frame_in_flight_index_ = 0;
}

void Context::GoToNextFrame() noexcept {
  ++frame_in_flight_index_;
  frame_in_flight_index_ %= max_frames_in_flight_;
  ++frame_count_;
}

FrameData& Context::GetFrameData(FrameInFlightIndex frame) {
  if (frame == kInvalidFrameInFlightIndex) {
    frame = frame_in_flight_index_;
  }

  COMET_ASSERT(frame < frame_data_.GetSize(), "Context::GetFrameData",
               "frame index is out of bounds", "frame", frame, "frame_count",
               frame_data_.GetSize());

  return frame_data_[frame];
}

const FrameData& Context::GetFrameData(FrameInFlightIndex frame) const {
  if (frame == kInvalidFrameInFlightIndex) {
    frame = frame_in_flight_index_;
  }

  COMET_ASSERT(frame < frame_data_.GetSize(), "Context::GetFrameData",
               "frame index is out of bounds", "frame", frame, "frame_count",
               frame_data_.GetSize());

  return frame_data_[frame];
}

bool Context::IsSamplerAnisotropy() const noexcept {
  return is_sampler_anisotropy_;
}

bool Context::IsSampleRateShading() const noexcept {
  return is_sample_rate_shading_;
}

ImageIndex Context::GetImageIndex() const {
  COMET_ASSERT(image_data_ != nullptr, "Context::GetImageIndex",
               "image data is null");
  return image_data_->image_index;
}

ImageIndex Context::GetImageCount() const {
  COMET_ASSERT(image_data_ != nullptr, "Context::GetImageCount",
               "image data is null");
  return image_data_->image_count;
}

VkSemaphore Context::GetRenderSemaphoreHandle() const {
  COMET_ASSERT(image_data_ != nullptr, "Context::GetRenderSemaphoreHandle",
               "image data is null");
  return image_data_->render_semaphore_handle;
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

VkCommandPool Context::GetGraphicsCommandPoolHandle(
    FrameInFlightIndex frame_index) const {
  return GetFrameData(frame_index).command_pool_handle;
}

VkCommandPool Context::GetUploadCommandPoolHandle(
    FrameInFlightIndex frame_index) const {
  if (frame_index == kInvalidFrameInFlightIndex) {
    frame_index = frame_in_flight_index_;
  }

  COMET_ASSERT(frame_index < frame_data_.GetSize(),
               "Context::GetUploadCommandPoolHandle",
               "frame index out of bounds", "frame_index", frame_index);

  if (device_->IsUploadQueueGraphics()) {
    return frame_data_[frame_index].command_pool_handle;
  }

  COMET_ASSERT(upload_command_pool_handle_ != VK_NULL_HANDLE,
               "Context::GetUploadCommandPoolHandle",
               "upload command pool handle is invalid");
  return upload_command_pool_handle_;
}

const QueueContext& Context::GetUploadQueueContext() const noexcept {
  return device_->GetUploadQueueContext();
}

const VkSemaphore* Context::GetUploadSemaphoreHandle() const {
  return &upload_semaphore_handle_;
}

VkQueue Context::GetGraphicsQueueHandle() const noexcept {
  return device_->GetGraphicsQueueContext().handle;
}

VkQueue Context::GetPresentQueueHandle() const noexcept {
  return device_->GetPresentQueueContext().handle;
}

VkQueue Context::GetTransferQueueHandle() const noexcept {
  return device_->GetTransferQueueContext().handle;
}

VkQueue Context::GetUploadQueueHandle() const noexcept {
  return device_->GetUploadQueueContext().handle;
}

u64 Context::GetUploadTimelineValue() const noexcept {
  return upload_timeline_value_;
}

u64 Context::AdvanceUploadTimelineValue() noexcept {
  ++upload_timeline_value_;
  return upload_timeline_value_;
}

VkCommandBuffer Context::GetUploadCommandBufferHandle(
    FrameInFlightIndex frame_index) const {
  if (frame_index == kInvalidFrameInFlightIndex) {
    frame_index = frame_in_flight_index_;
  }

  COMET_ASSERT(frame_index < frame_data_.GetSize(),
               "Context::GetUploadCommandBufferHandle",
               "frame index out of bounds", "frame_index", frame_index);

  return frame_data_[frame_index].upload_command_buffer_handle;
}

VkFence Context::GetUploadFenceHandle(FrameInFlightIndex frame_index) const {
  if (frame_index == kInvalidFrameInFlightIndex) {
    frame_index = frame_in_flight_index_;
  }

  COMET_ASSERT(frame_index < frame_data_.GetSize(),
               "Context::GetUploadFenceHandle", "frame index out of bounds",
               "frame_index", frame_index);

  return frame_data_[frame_index].upload_fence_handle;
}

bool Context::IsInitialized() const noexcept { return is_initialized_; }
}  // namespace vk
}  // namespace rendering
}  // namespace comet