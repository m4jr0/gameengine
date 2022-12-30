// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_COMMON_TYPES_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_COMMON_TYPES_H_

#include "comet_precompile.h"

#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/vulkan_descriptor.h"
#include "comet/rendering/driver/vulkan/vulkan_device.h"

namespace comet {
namespace rendering {
namespace vk {
struct FrameData {
  VkCommandPool command_pool{VK_NULL_HANDLE};
  VkCommandBuffer command_buffer{VK_NULL_HANDLE};

  AllocatedBuffer buffer{};
  VkDescriptorSet global_descriptor_set{VK_NULL_HANDLE};

  AllocatedBuffer object_buffer{};
  VkDescriptorSet object_descriptor_set{VK_NULL_HANDLE};

  VulkanDescriptorAllocator descriptor_allocator{};

  VkSemaphore present_semaphore{VK_NULL_HANDLE};
  VkSemaphore render_semaphore{VK_NULL_HANDLE};
  VkFence render_fence{VK_NULL_HANDLE};
};

struct UploadContext {
  VkFence upload_fence{VK_NULL_HANDLE};
  VkCommandPool command_pool{VK_NULL_HANDLE};
  VkCommandBuffer command_buffer{VK_NULL_HANDLE};
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

struct SceneData {
  alignas(16) glm::vec4 fog_color;
  alignas(16) glm::vec4 fog_distances;
  alignas(16) glm::vec4 ambient_color;
  alignas(16) glm::vec4 sunlight_direction;
  alignas(16) glm::vec4 sunlight_color;
};

struct CameraData {
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
  alignas(16) glm::mat4 view_proj;
};

struct MeshPushConstants {
  alignas(16) glm::mat4 render_matrix;
  alignas(16) glm::vec4 data;
};

struct ObjectData {
  alignas(16) glm::mat4 model;
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_COMMON_TYPES_H_
