// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_TYPES_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_TYPES_H_

#include "comet_precompile.h"

#include <set>

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

namespace comet {
namespace rendering {
namespace vk {
using PhysicalDeviceScore = u32;

struct AllocatedBuffer {
  VkBuffer buffer{VK_NULL_HANDLE};
  VmaAllocation allocation{VK_NULL_HANDLE};
};

struct AllocatedImage {
  VkImage image{VK_NULL_HANDLE};
  VmaAllocation allocation{VK_NULL_HANDLE};
};

struct FrameData {
  VkCommandPool command_pool{VK_NULL_HANDLE};
  VkCommandBuffer command_buffer{VK_NULL_HANDLE};

  VkSemaphore present_semaphore{VK_NULL_HANDLE};
  VkSemaphore render_semaphore{VK_NULL_HANDLE};
  VkFence render_fence{VK_NULL_HANDLE};
};

struct UploadContext {
  VkFence upload_fence{VK_NULL_HANDLE};
  VkCommandPool command_pool{VK_NULL_HANDLE};
  VkCommandBuffer command_buffer{VK_NULL_HANDLE};
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

struct QueueFamilyIndices {
  std::optional<u32> graphics_family;
  std::optional<u32> present_family;
  std::optional<u32> transfer_family;

  bool IsComplete();
  bool IsSpecificTransferFamily();
  std::vector<u32> GetUniqueIndices();
};

struct PhysicalDeviceDescr {
  VkPhysicalDevice device{VK_NULL_HANDLE};
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  PhysicalDeviceScore score;
  QueueFamilyIndices queue_family_indices;
  VkSampleCountFlagBits msaa_samples{VK_SAMPLE_COUNT_1_BIT};
};

class CommandBuffer {
 public:
  CommandBuffer(VkDevice device, VkCommandPool command_pool,
                VkCommandBuffer command_buffer = VK_NULL_HANDLE);
  CommandBuffer(const CommandBuffer&) = delete;
  CommandBuffer(CommandBuffer&&) noexcept = default;
  CommandBuffer& operator=(const CommandBuffer&) = delete;
  CommandBuffer& operator=(CommandBuffer&&) noexcept = default;
  ~CommandBuffer();

  void Allocate();
  void Record();
  void Submit(VkQueue queue);
  void Free();

  VkCommandBuffer GetHandle() const noexcept;
  VkCommandPool GetPool() const noexcept;
  VkDevice GetDevice() const noexcept;

 private:
  VkDevice device_{VK_NULL_HANDLE};
  VkCommandBuffer command_buffer_{VK_NULL_HANDLE};
  VkCommandPool command_pool_{VK_NULL_HANDLE};
  bool is_allocated_{false};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_TYPES_H_
