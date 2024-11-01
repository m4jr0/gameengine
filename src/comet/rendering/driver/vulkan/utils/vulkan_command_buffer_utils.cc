// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_command_buffer_utils.h"

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
               "Tried to allocate command data, but it is already allocated!");
  auto alloc_info{init::GenerateCommandBufferAllocateInfo(
      command_data.command_pool_handle, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY)};
  COMET_CHECK_VK(
      vkAllocateCommandBuffers(command_data.device_handle, &alloc_info,
                               &command_data.command_buffer_handle),
      "Failed to allocate command buffer");
  command_data.is_allocated = true;
  COMET_VK_SET_DEBUG_LABEL(
      command_data.command_buffer_handle,
      debug_label != nullptr ? debug_label : "command_data");
}

void DestroyCommandData(CommandData& command_data) {
  COMET_ASSERT(command_data.is_allocated,
               "Tried to destroy command data, but it is not allocated!");
  vkFreeCommandBuffers(command_data.device_handle,
                       command_data.command_pool_handle, 1,
                       &command_data.command_buffer_handle);
  command_data.command_buffer_handle = VK_NULL_HANDLE;
  command_data.is_allocated = false;
}

void RecordCommand(VkCommandBuffer command_buffer_handle) {
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags =
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // For now, it's a one-time
                                                    // command.
  COMET_CHECK_VK(vkBeginCommandBuffer(command_buffer_handle, &begin_info),
                 "Failed to begin recording command buffer");
}

void RecordCommand(const CommandData& command_data) {
  RecordCommand(command_data.command_buffer_handle);
}

void SubmitCommand(VkCommandBuffer command_buffer_handle, VkQueue queue_handle,
                   VkFence fence_handle, const VkSemaphore* wait_semaphore,
                   const VkSemaphore* signal_semaphore,
                   const VkPipelineStageFlags* wait_dst_stage_mask) {
  vkEndCommandBuffer(command_buffer_handle);
  VkSubmitInfo submit_info{
      init::GenerateSubmitInfo(&command_buffer_handle, wait_semaphore,
                               signal_semaphore, wait_dst_stage_mask)};

  vkQueueSubmit(queue_handle, 1, &submit_info, fence_handle);
}

void SubmitCommand(const CommandData& command_data, VkQueue queue_handle,
                   VkFence fence_handle, const VkSemaphore* wait_semaphore,
                   const VkSemaphore* signal_semaphore,
                   const VkPipelineStageFlags* wait_dst_stage_mask) {
  SubmitCommand(command_data.command_buffer_handle, queue_handle, fence_handle,
                wait_semaphore, signal_semaphore, wait_dst_stage_mask);
}

VkCommandBuffer GenerateOneTimeCommand(VkDevice device_handle,
                                       VkCommandPool command_pool_handle) {
  VkCommandBufferAllocateInfo alloc_info{
      init::GenerateCommandBufferAllocateInfo(command_pool_handle, 1)};

  VkCommandBuffer command_buffer_handle;

  COMET_CHECK_VK(vkAllocateCommandBuffers(device_handle, &alloc_info,
                                          &command_buffer_handle),
                 "Could not allocate command buffer!");

  RecordCommand(command_buffer_handle);
  return command_buffer_handle;
}

void SubmitOneTimeCommand(VkCommandBuffer& command_buffer_handle,
                          VkCommandPool command_pool_handle,
                          VkDevice device_handle, VkQueue queue_handle) {
  SubmitCommand(command_buffer_handle, queue_handle);
  vkQueueWaitIdle(queue_handle);
  vkFreeCommandBuffers(device_handle, command_pool_handle, 1,
                       &command_buffer_handle);
  command_buffer_handle = VK_NULL_HANDLE;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet