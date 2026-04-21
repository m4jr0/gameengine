// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_command_buffer_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
CommandData GenerateCommandData(VkDevice device_handle,
                                VkCommandPool command_pool_handle) {
  CommandData command_data{};
  command_data.device_handle = device_handle;
  command_data.command_pool_handle = command_pool_handle;
  return command_data;
}

CommandData GenerateCommandData(VkDevice device_handle,
                                VkCommandBuffer command_buffer_handle) {
  CommandData command_data{};
  command_data.device_handle = device_handle;
  command_data.command_buffer_handle = command_buffer_handle;
  command_data.is_allocated = command_buffer_handle != VK_NULL_HANDLE;
  return command_data;
}

void AllocateCommandData(CommandData& command_data,
                         [[maybe_unused]] const schar* debug_label) {
  COMET_ASSERT(!command_data.is_allocated,
               "vulkan_command_buffer_utils::AllocateCommandData",
               "command data is already allocated");
  COMET_ASSERT(command_data.device_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::AllocateCommandData",
               "device handle is invalid");
  COMET_ASSERT(command_data.command_pool_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::AllocateCommandData",
               "command pool handle is invalid");

  const auto alloc_info{init::GenerateCommandBufferAllocateInfo(
      command_data.command_pool_handle, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY)};

  COMET_CHECK_VK(
      vkAllocateCommandBuffers(command_data.device_handle, &alloc_info,
                               &command_data.command_buffer_handle),
      "vulkan_command_buffer_utils::AllocateCommandData",
      "command buffer allocation failed");

  command_data.is_allocated = true;
  COMET_VK_SET_DEBUG_LABEL(
      command_data.command_buffer_handle,
      debug_label != nullptr ? debug_label : "command_data");
}

void DestroyCommandData(CommandData& command_data) {
  COMET_ASSERT(command_data.is_allocated,
               "vulkan_command_buffer_utils::DestroyCommandData",
               "command data is not allocated");
  COMET_ASSERT(command_data.device_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::DestroyCommandData",
               "device handle is invalid");
  COMET_ASSERT(command_data.command_pool_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::DestroyCommandData",
               "command pool handle is invalid");
  COMET_ASSERT(command_data.command_buffer_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::DestroyCommandData",
               "command buffer handle is invalid");

  vkFreeCommandBuffers(command_data.device_handle,
                       command_data.command_pool_handle, 1,
                       &command_data.command_buffer_handle);
  command_data.command_buffer_handle = VK_NULL_HANDLE;
  command_data.is_allocated = false;
}

void RecordCommand(VkCommandBuffer command_buffer_handle) {
  COMET_ASSERT(command_buffer_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::RecordCommand",
               "command buffer handle is invalid");

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags =
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // For now, it's a one-time
                                                    // command.

  COMET_CHECK_VK(vkBeginCommandBuffer(command_buffer_handle, &begin_info),
                 "vulkan_command_buffer_utils::RecordCommand",
                 "command buffer begin failed");
}

void BeginFrameCommandRecording(const CommandData& command_data) {
  RecordCommand(command_data.command_buffer_handle);
}

void SubmitCommand(VkCommandBuffer command_buffer_handle, VkQueue queue_handle,
                   VkFence fence_handle, const VkSemaphore* wait_semaphores,
                   u32 wait_semaphore_count,
                   const VkSemaphore* signal_semaphores,
                   u32 signal_semaphore_count,
                   const VkPipelineStageFlags* wait_dst_stage_mask,
                   const void* next) {
  COMET_ASSERT(command_buffer_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::SubmitCommand",
               "command buffer handle is invalid");
  COMET_ASSERT(queue_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::SubmitCommand",
               "queue handle is invalid");

  COMET_CHECK_VK(vkEndCommandBuffer(command_buffer_handle),
                 "vulkan_command_buffer_utils::SubmitCommand",
                 "command buffer end failed");

  const auto submit_info{init::GenerateSubmitInfo(
      &command_buffer_handle, wait_semaphores, wait_semaphore_count,
      signal_semaphores, signal_semaphore_count, wait_dst_stage_mask, next)};

  COMET_CHECK_VK(vkQueueSubmit(queue_handle, 1, &submit_info, fence_handle),
                 "vulkan_command_buffer_utils::SubmitCommand",
                 "queue submit failed");
}

void SubmitCommand(const CommandData& command_data, VkQueue queue_handle,
                   VkFence fence_handle, const VkSemaphore* wait_semaphores,
                   u32 wait_semaphore_count,
                   const VkSemaphore* signal_semaphores,
                   u32 signal_semaphore_count,
                   const VkPipelineStageFlags* wait_dst_stage_mask,
                   const void* next) {
  SubmitCommand(command_data.command_buffer_handle, queue_handle, fence_handle,
                wait_semaphores, wait_semaphore_count, signal_semaphores,
                signal_semaphore_count, wait_dst_stage_mask, next);
}

void SubmitCommand2(u32 command_buffer_info_count,
                    const VkCommandBufferSubmitInfo* command_buffer_infos,
                    VkQueue queue_handle, VkFence fence_handle,
                    const VkSemaphoreSubmitInfo* wait_semaphore_infos,
                    u32 wait_semaphore_info_count,
                    const VkSemaphoreSubmitInfo* signal_semaphore_infos,
                    u32 signal_semaphore_info_count, const void* next) {
  COMET_ASSERT(command_buffer_infos != nullptr,
               "vulkan_command_buffer_utils::SubmitCommand2",
               "command buffer submit infos are null");
  COMET_ASSERT(command_buffer_info_count > 0,
               "vulkan_command_buffer_utils::SubmitCommand2",
               "command buffer submit info count is zero");
  COMET_ASSERT(queue_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::SubmitCommand2",
               "queue handle is invalid");

  for (u32 i{0}; i < command_buffer_info_count; ++i) {
    const auto& info{command_buffer_infos[i]};
    COMET_ASSERT(info.commandBuffer != VK_NULL_HANDLE,
                 "vulkan_command_buffer_utils::SubmitCommand2",
                 "command buffer handle is invalid", "index", i);

    COMET_CHECK_VK(vkEndCommandBuffer(info.commandBuffer),
                   "vulkan_command_buffer_utils::SubmitCommand2",
                   "command buffer end failed");
  }

  const auto submit_info{init::GenerateSubmitInfo2(
      command_buffer_info_count, command_buffer_infos, wait_semaphore_infos,
      wait_semaphore_info_count, signal_semaphore_infos,
      signal_semaphore_info_count, next)};

  COMET_CHECK_VK(vkQueueSubmit2(queue_handle, 1, &submit_info, fence_handle),
                 "vulkan_command_buffer_utils::SubmitCommand2",
                 "queue submit failed");
}

void SubmitCommand2(const CommandData& command_data, VkQueue queue_handle,
                    VkFence fence_handle,
                    const VkSemaphoreSubmitInfo* wait_semaphore_infos,
                    u32 wait_semaphore_info_count,
                    const VkSemaphoreSubmitInfo* signal_semaphore_infos,
                    u32 signal_semaphore_info_count, const void* next) {
  VkCommandBufferSubmitInfo command_buffer_info{};

  command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  command_buffer_info.commandBuffer = command_data.command_buffer_handle;
  command_buffer_info.deviceMask = 0;

  SubmitCommand2(1, &command_buffer_info, queue_handle, fence_handle,
                 wait_semaphore_infos, wait_semaphore_info_count,
                 signal_semaphore_infos, signal_semaphore_info_count, next);
}

VkCommandBuffer GenerateOneTimeCommand(VkDevice device_handle,
                                       VkCommandPool command_pool_handle) {
  COMET_ASSERT(device_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::GenerateOneTimeCommand",
               "device handle is invalid");
  COMET_ASSERT(command_pool_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::GenerateOneTimeCommand",
               "command pool handle is invalid");

  VkCommandBufferAllocateInfo alloc_info{
      init::GenerateCommandBufferAllocateInfo(command_pool_handle, 1)};

  VkCommandBuffer command_buffer_handle;

  COMET_CHECK_VK(vkAllocateCommandBuffers(device_handle, &alloc_info,
                                          &command_buffer_handle),
                 "vulkan_command_buffer_utils::GenerateOneTimeCommand",
                 "command buffer allocation failed");

  RecordCommand(command_buffer_handle);
  return command_buffer_handle;
}

void SubmitOneTimeCommand(
    VkCommandBuffer& command_buffer_handle, VkCommandPool command_pool_handle,
    VkDevice device_handle, VkQueue queue_handle, VkFence fence_handle,
    const VkSemaphore* wait_semaphore, const VkSemaphore* signal_semaphore,
    const VkPipelineStageFlags* wait_dst_stage_mask, const void* next) {
  COMET_ASSERT(command_buffer_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::SubmitOneTimeCommand",
               "command buffer handle is invalid");
  COMET_ASSERT(command_pool_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::SubmitOneTimeCommand",
               "command pool handle is invalid");
  COMET_ASSERT(device_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::SubmitOneTimeCommand",
               "device handle is invalid");
  COMET_ASSERT(queue_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::SubmitOneTimeCommand",
               "queue handle is invalid");

  auto used_fence_handle{fence_handle};

  if (used_fence_handle == VK_NULL_HANDLE) {
    const auto fence_info{init::GenerateFenceCreateInfo()};

    COMET_CHECK_VK(vkCreateFence(device_handle, &fence_info, VK_NULL_HANDLE,
                                 &used_fence_handle),
                   "vulkan_command_buffer_utils::SubmitOneTimeCommand",
                   "fence creation failed");
  }

  SubmitCommand(command_buffer_handle, queue_handle, used_fence_handle,
                wait_semaphore, wait_semaphore != VK_NULL_HANDLE ? 1 : 0,
                signal_semaphore, signal_semaphore != VK_NULL_HANDLE ? 1 : 0,
                wait_dst_stage_mask, next);

  COMET_CHECK_VK(vkWaitForFences(device_handle, 1, &used_fence_handle, VK_TRUE,
                                 UINT64_MAX),
                 "vulkan_command_buffer_utils::SubmitOneTimeCommand",
                 "fence wait failed");

  vkFreeCommandBuffers(device_handle, command_pool_handle, 1,
                       &command_buffer_handle);

  if (fence_handle == VK_NULL_HANDLE) {
    vkDestroyFence(device_handle, used_fence_handle, VK_NULL_HANDLE);
  } else {
    COMET_CHECK_VK(vkResetFences(device_handle, 1, &fence_handle),
                   "vulkan_command_buffer_utils::SubmitOneTimeCommand",
                   "fence reset failed");
  }

  command_buffer_handle = VK_NULL_HANDLE;
}

void SubmitOneTimeCommandAsync(VkCommandBuffer command_buffer_handle,
                               VkQueue queue_handle, VkFence fence_handle,
                               const VkSemaphore* wait_semaphore,
                               const VkSemaphore* signal_semaphore,
                               const VkPipelineStageFlags* wait_dst_stage_mask,
                               const void* next) {
  COMET_ASSERT(command_buffer_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::SubmitOneTimeCommandAsync",
               "command buffer handle is invalid");
  COMET_ASSERT(queue_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::SubmitOneTimeCommandAsync",
               "queue handle is invalid");

  SubmitCommand(command_buffer_handle, queue_handle, fence_handle,
                wait_semaphore, wait_semaphore != VK_NULL_HANDLE ? 1 : 0,
                signal_semaphore, signal_semaphore != VK_NULL_HANDLE ? 1 : 0,
                wait_dst_stage_mask, next);
}

void WaitAndRecycleOneTimeCommand(VkDevice device_handle,
                                  VkCommandPool command_pool_handle,
                                  VkCommandBuffer& command_buffer_handle,
                                  VkFence fence_handle) {
  COMET_ASSERT(device_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::WaitAndRecycleOneTimeCommand",
               "device handle is invalid");
  COMET_ASSERT(command_pool_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::WaitAndRecycleOneTimeCommand",
               "command pool handle is invalid");
  COMET_ASSERT(command_buffer_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::WaitAndRecycleOneTimeCommand",
               "command buffer handle is invalid");
  COMET_ASSERT(fence_handle != VK_NULL_HANDLE,
               "vulkan_command_buffer_utils::WaitAndRecycleOneTimeCommand",
               "fence handle is invalid");

  COMET_CHECK_VK(
      vkWaitForFences(device_handle, 1, &fence_handle, VK_TRUE, UINT64_MAX),
      "vulkan_command_buffer_utils::WaitAndRecycleOneTimeCommand",
      "fence wait failed");

  vkFreeCommandBuffers(device_handle, command_pool_handle, 1,
                       &command_buffer_handle);

  COMET_CHECK_VK(vkResetFences(device_handle, 1, &fence_handle),
                 "vulkan_command_buffer_utils::WaitAndRecycleOneTimeCommand",
                 "fence reset failed");

  command_buffer_handle = VK_NULL_HANDLE;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet