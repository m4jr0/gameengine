// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_common_types.h"

#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool command_pool,
                             VkCommandBuffer command_buffer)
    : device_{device},
      command_pool_{command_pool},
      command_buffer_{command_buffer} {}

CommandBuffer::~CommandBuffer() {
  if (is_allocated_) {
    Free();
  }
}

void CommandBuffer::Allocate() {
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandPool = command_pool_;
  alloc_info.commandBufferCount = 1;

  COMET_CHECK_VK(
      vkAllocateCommandBuffers(device_, &alloc_info, &command_buffer_),
      "Failed to allocate command buffer");

  is_allocated_ = true;
}

void CommandBuffer::Record() {
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags =
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // For now, it's a one-time
                                                    // command.

  COMET_CHECK_VK(vkBeginCommandBuffer(command_buffer_, &begin_info),
                 "Failed to begin recording command buffer");
}

void CommandBuffer::Submit(VkQueue queue) {
  vkEndCommandBuffer(command_buffer_);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer_;

  vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(queue);
}

void CommandBuffer::Free() {
  vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer_);
  is_allocated_ = false;
}

VkCommandBuffer CommandBuffer::GetHandle() const noexcept {
  return command_buffer_;
}

VkCommandPool CommandBuffer::GetPool() const noexcept { return command_pool_; }

VkDevice CommandBuffer::GetDevice() const noexcept { return device_; }
}  // namespace vk
}  // namespace rendering
}  // namespace comet