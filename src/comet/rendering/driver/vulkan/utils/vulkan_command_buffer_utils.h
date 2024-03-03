// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_COMMAND_BUFFER_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_COMMAND_BUFFER_UTILS_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/data/vulkan_command_buffer.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"

namespace comet {
namespace rendering {
namespace vk {
CommandData GenerateCommandData(VkDevice device_handle,
                                VkCommandPool command_pool_handle);
CommandData GenerateCommandData(VkDevice device_handle,
                                VkCommandBuffer command_buffer_handle);
void AllocateCommandData(CommandData& command_data);
void DestroyCommandData(CommandData& command_data);
void RecordCommand(VkCommandBuffer command_buffer_handle);
void RecordCommand(const CommandData& command_data);
void SubmitCommand(
    VkDevice device_handle, VkCommandBuffer command_buffer_handle,
    VkQueue queue_handle, VkFence fence_handle = VK_NULL_HANDLE,
    const VkSemaphore* wait_semaphore = VK_NULL_HANDLE,
    const VkSemaphore* signal_semaphore = VK_NULL_HANDLE,
    const VkPipelineStageFlags* wait_dst_stage_mask = VK_NULL_HANDLE);
void SubmitCommand(
    const CommandData& command_data, VkQueue queue_handle,
    VkFence fence_handle = VK_NULL_HANDLE,
    const VkSemaphore* wait_semaphore = VK_NULL_HANDLE,
    const VkSemaphore* signal_semaphore = VK_NULL_HANDLE,
    const VkPipelineStageFlags* wait_dst_stage_mask = VK_NULL_HANDLE);
VkCommandBuffer GenerateOneTimeCommand(VkDevice device_handle,
                                       VkCommandPool command_pool_handle);
void SubmitOneTimeCommand(VkCommandBuffer& command_buffer_handle,
                          VkCommandPool command_pool_handle,
                          VkDevice device_handle, VkQueue queue_handle);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_UTILS_VULKAN_COMMAND_BUFFER_UTILS_H_
