// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_types.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace rendering {
namespace vk {
bool QueueFamilyIndices::IsComplete() {
  return graphics_family.has_value() && present_family.has_value() &&
         transfer_family.has_value();
}

bool QueueFamilyIndices::IsSpecificTransferFamily() {
  if (!graphics_family.has_value() || !transfer_family.has_value()) {
    return false;
  }

  return graphics_family.value() != transfer_family.value();
}

std::vector<std::uint32_t> QueueFamilyIndices::GetUniqueIndices() {
  if (!IsComplete()) {
    throw std::runtime_error("Queue Family Indices is not complete");
  }

  auto graphics_fam_index = graphics_family.value();
  auto present_fam_index = present_family.value();
  auto transfer_fam_index = transfer_family.value();

  std::set<std::uint32_t> set = {
      graphics_family.value(), present_family.value(), transfer_family.value()};

  std::vector<std::uint32_t> list{};
  list.reserve(set.size());

  for (auto it = set.begin(); it != set.end();) {
    list.push_back(std::move(set.extract(it++).value()));
  }

  return list;
}

CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool command_pool,
                             VkCommandBuffer command_buffer)
    : device_(device),
      command_pool_(command_pool),
      command_buffer_(command_buffer) {}

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

  if (vkAllocateCommandBuffers(device_, &alloc_info, &command_buffer_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffer");
  }

  is_allocated_ = true;
}

void CommandBuffer::Record() {
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags =
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // For now, it's a one-time
                                                    // command.

  if (vkBeginCommandBuffer(command_buffer_, &begin_info) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer");
  }
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