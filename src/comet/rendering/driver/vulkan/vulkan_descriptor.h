// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DESCRIPTOR_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DESCRIPTOR_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

namespace comet {
namespace rendering {
namespace vk {
class VulkanDescriptorAllocator {
 public:
  VulkanDescriptorAllocator() = default;
  VulkanDescriptorAllocator(const VulkanDescriptorAllocator&) = delete;
  VulkanDescriptorAllocator(VulkanDescriptorAllocator&&) = delete;
  VulkanDescriptorAllocator& operator=(const VulkanDescriptorAllocator&) =
      delete;
  VulkanDescriptorAllocator& operator=(VulkanDescriptorAllocator&&) = delete;
  ~VulkanDescriptorAllocator() = default;

  void Destroy();

  bool Allocate(VkDescriptorSet& set, VkDescriptorSetLayout layout,
                bool is_retry = true);
  void ResetPools();

  void SetDevice(VkDevice device) noexcept;
  VkDevice GetDevice() const noexcept;

 private:
  static constexpr u32 kDefaultMaxSetCount_{1000};
  static constexpr std::array<VkDescriptorPoolSize, 11> kDefaultPoolSizes_{
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, 500},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4000},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4000},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2000},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2000},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 500}};

  VkDevice device_{VK_NULL_HANDLE};
  VkDescriptorPool current_pool_{VK_NULL_HANDLE};
  std::vector<VkDescriptorPool> used_pools_;
  std::vector<VkDescriptorPool> available_pools_;

  void UpdateCurrentPool();
};

class VulkanDescriptorSetLayoutHandler {
 public:
  VulkanDescriptorSetLayoutHandler() = default;
  VulkanDescriptorSetLayoutHandler(const VulkanDescriptorSetLayoutHandler&) =
      delete;
  VulkanDescriptorSetLayoutHandler(VulkanDescriptorSetLayoutHandler&&) = delete;
  VulkanDescriptorSetLayoutHandler& operator=(
      const VulkanDescriptorSetLayoutHandler&) = delete;
  VulkanDescriptorSetLayoutHandler& operator=(
      VulkanDescriptorSetLayoutHandler&&) = delete;
  ~VulkanDescriptorSetLayoutHandler() = default;

  void Destroy();

  VkDescriptorSetLayout Get(VkDescriptorSetLayoutCreateInfo& info);

  void SetDevice(VkDevice device) noexcept;
  VkDevice GetDevice() const noexcept;

  struct VulkanDescriptorSetLayoutDescr {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bool operator==(const VulkanDescriptorSetLayoutDescr& other) const;
    std::size_t hash() const;
  };

 private:
  struct VulkanDescriptorSetLayoutHash {
    std::size_t operator()(const VulkanDescriptorSetLayoutDescr& info) const;
  };

  std::unordered_map<VulkanDescriptorSetLayoutDescr, VkDescriptorSetLayout,
                     VulkanDescriptorSetLayoutHash>
      cache_;
  VkDevice device_{VK_NULL_HANDLE};
};

struct VulkanDescriptorBuilderDescr {
  VulkanDescriptorSetLayoutHandler* handler;
  VulkanDescriptorAllocator* allocator;
};

class VulkanDescriptorBuilder {
 public:
  VulkanDescriptorBuilder(VulkanDescriptorSetLayoutHandler& handler,
                          VulkanDescriptorAllocator& allocator);
  VulkanDescriptorBuilder(const VulkanDescriptorBuilder&) = delete;
  VulkanDescriptorBuilder(VulkanDescriptorBuilder&&) = delete;
  VulkanDescriptorBuilder& operator=(const VulkanDescriptorBuilder&) = delete;
  VulkanDescriptorBuilder& operator=(VulkanDescriptorBuilder&&) = delete;
  ~VulkanDescriptorBuilder() = default;

  static VulkanDescriptorBuilder Generate(
      VulkanDescriptorSetLayoutHandler& handler,
      VulkanDescriptorAllocator& allocator);

  static VulkanDescriptorBuilder Generate(VulkanDescriptorBuilderDescr& descr);

  VulkanDescriptorBuilder& Bind(u32 binding,
                                VkDescriptorBufferInfo& buffer_info,
                                VkDescriptorType type,
                                VkShaderStageFlags stage_flags);

  VulkanDescriptorBuilder& Bind(u32 binding, VkDescriptorImageInfo& image_info,
                                VkDescriptorType type,
                                VkShaderStageFlags stage_flags);

  void Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
  void Build(VkDescriptorSet& set);

 private:
  std::vector<VkWriteDescriptorSet> writes_;
  std::vector<VkDescriptorSetLayoutBinding> bindings_;
  VulkanDescriptorSetLayoutHandler* handler_{nullptr};
  VulkanDescriptorAllocator* allocator_{nullptr};
};

std::size_t GetVulkanDescriptorSetLayoutBindingHash(
    const VkDescriptorSetLayoutBinding& binding);

std::size_t GetVulkanDescriptorSetLayoutCreateInfoHash(
    const VkDescriptorSetLayoutCreateInfo& info);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DESCRIPTOR_H_
