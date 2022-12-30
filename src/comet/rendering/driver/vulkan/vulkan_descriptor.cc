// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_descriptor.h"

#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/driver/vulkan/vulkan_initializers.h"
#include "comet/utils/hash.h"

namespace comet {
namespace rendering {
namespace vk {
void VulkanDescriptorAllocator::Destroy() {
  COMET_ASSERT(device_ != VK_NULL_HANDLE, "No device provided!");

  for (auto pool : used_pools_) {
    vkDestroyDescriptorPool(device_, pool, VK_NULL_HANDLE);
  }

  for (auto pool : available_pools_) {
    vkDestroyDescriptorPool(device_, pool, VK_NULL_HANDLE);
  }

  used_pools_.clear();
  available_pools_.clear();
  current_pool_ = VK_NULL_HANDLE;
}

bool VulkanDescriptorAllocator::Allocate(VkDescriptorSet& set,
                                         VkDescriptorSetLayout layout,
                                         bool is_retry) {
  if (current_pool_ == VK_NULL_HANDLE) {
    UpdateCurrentPool();
  }

  COMET_ASSERT(device_ != VK_NULL_HANDLE, "No device provided!");

  VkDescriptorSetAllocateInfo allocate_info{};
  allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocate_info.pSetLayouts = &layout;
  allocate_info.descriptorSetCount = 1;
  allocate_info.descriptorPool = current_pool_;
  allocate_info.pNext = VK_NULL_HANDLE;

  const auto result{vkAllocateDescriptorSets(device_, &allocate_info, &set)};

  switch (result) {
    case VK_SUCCESS:
      return true;

    case VK_ERROR_FRAGMENTED_POOL:
    case VK_ERROR_OUT_OF_POOL_MEMORY:
      if (is_retry) {
        UpdateCurrentPool();
        return Allocate(set, layout, false);
      }

      COMET_ASSERT(false, "Could not allocate descriptor sets!");
      return false;
  }

  COMET_ASSERT(false, "Could not allocate descriptor sets!");
  return false;
}

void VulkanDescriptorAllocator::ResetPools() {
  for (auto pool : available_pools_) {
    vkDestroyDescriptorPool(device_, pool, VK_NULL_HANDLE);
  }

  for (auto pool : used_pools_) {
    vkResetDescriptorPool(device_, pool, 0);
  }

  available_pools_ = std::move(used_pools_);
  used_pools_ = std::vector<VkDescriptorPool>();
  current_pool_ = VK_NULL_HANDLE;
}

void VulkanDescriptorAllocator::SetDevice(VkDevice device) noexcept {
  device_ = device;
}

VkDevice VulkanDescriptorAllocator::GetDevice() const noexcept {
  return device_;
}

void VulkanDescriptorAllocator::UpdateCurrentPool() {
  if (available_pools_.size() > 0) {
    auto pool{available_pools_.back()};
    available_pools_.pop_back();
    current_pool_ = pool;
  } else {
    COMET_ASSERT(device_ != VK_NULL_HANDLE, "No device provided!");

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = kDefaultMaxSetCount_;
    pool_info.poolSizeCount = static_cast<u32>(kDefaultPoolSizes_.size());
    pool_info.pPoolSizes = kDefaultPoolSizes_.data();

    COMET_CHECK_VK(vkCreateDescriptorPool(device_, &pool_info, VK_NULL_HANDLE,
                                          &current_pool_),
                   "Failed to create descriptor pool!");
  }

  used_pools_.push_back(current_pool_);
}

void VulkanDescriptorSetLayoutHandler::Destroy() {
  COMET_ASSERT(device_ != VK_NULL_HANDLE, "No device provided!");

  for (auto& pair : cache_) {
    if (pair.second == VK_NULL_HANDLE) {
      continue;
    }

    vkDestroyDescriptorSetLayout(device_, pair.second, VK_NULL_HANDLE);
  }

  cache_.clear();
}

VkDescriptorSetLayout VulkanDescriptorSetLayoutHandler::Get(
    VkDescriptorSetLayoutCreateInfo& info) {
  VulkanDescriptorSetLayoutDescr descr;
  descr.bindings.resize(info.bindingCount);
  bool is_sorted{true};

  for (uindex i{0}; i < info.bindingCount; ++i) {
    descr.bindings[i] = info.pBindings[i];

    if (i == 0 || !is_sorted) {
      continue;
    }

    if (descr.bindings[i].binding < descr.bindings[i - 1].binding) {
      is_sorted = false;
    }
  }

  if (!is_sorted) {
    std::sort(descr.bindings.begin(), descr.bindings.end(),
              [](VkDescriptorSetLayoutBinding& binding1,
                 VkDescriptorSetLayoutBinding& binding2) {
                return binding1.binding < binding2.binding;
              });
  }

  const auto it{cache_.find(descr)};

  if (it != cache_.cend()) {
    return it->second;
  }

  COMET_ASSERT(device_ != VK_NULL_HANDLE, "No device provided!");
  VkDescriptorSetLayout layout;
  vkCreateDescriptorSetLayout(device_, &info, VK_NULL_HANDLE, &layout);
  cache_[descr] = layout;
  return layout;
}

void VulkanDescriptorSetLayoutHandler::SetDevice(VkDevice device) noexcept {
  device_ = device;
}

VkDevice VulkanDescriptorSetLayoutHandler::GetDevice() const noexcept {
  return device_;
}

bool VulkanDescriptorSetLayoutHandler::VulkanDescriptorSetLayoutDescr::
operator==(const VulkanDescriptorSetLayoutDescr& other) const {
  const auto binding_count{bindings.size()};

  if (binding_count != other.bindings.size()) {
    return false;
  }

  for (uindex i{0}; i < binding_count; ++i) {
    const auto& binding{bindings[i]};
    const auto& other_binding{other.bindings[i]};

    if (binding.binding != other_binding.binding ||
        binding.descriptorType != other_binding.descriptorType ||
        binding.descriptorCount != other_binding.descriptorCount ||
        binding.stageFlags != other_binding.stageFlags) {
      return false;
    }
  }

  return true;
}

std::size_t
VulkanDescriptorSetLayoutHandler::VulkanDescriptorSetLayoutDescr::hash() const {
  auto hash{std::hash<std::size_t>()(bindings.size())};

  for (const auto& binding : bindings) {
    std::size_t binding_hash{GetVulkanDescriptorSetLayoutBindingHash(binding)};
    hash =
        utils::hash::HashCombine(hash, std::hash<std::size_t>()(binding_hash));
  }

  return hash;
}

std::size_t
VulkanDescriptorSetLayoutHandler::VulkanDescriptorSetLayoutHash::operator()(
    const VulkanDescriptorSetLayoutDescr& info) const {
  return info.hash();
}

VulkanDescriptorBuilder::VulkanDescriptorBuilder(
    VulkanDescriptorSetLayoutHandler& handler,
    VulkanDescriptorAllocator& allocator)
    : handler_{&handler}, allocator_{&allocator} {}

VulkanDescriptorBuilder VulkanDescriptorBuilder::Generate(
    VulkanDescriptorSetLayoutHandler& handler,
    VulkanDescriptorAllocator& allocator) {
  return VulkanDescriptorBuilder{handler, allocator};
}

VulkanDescriptorBuilder VulkanDescriptorBuilder::Generate(
    VulkanDescriptorBuilderDescr& descr) {
  return VulkanDescriptorBuilder{*descr.handler, *descr.allocator};
}

VulkanDescriptorBuilder& VulkanDescriptorBuilder::Bind(
    u32 binding, VkDescriptorBufferInfo& buffer_info, VkDescriptorType type,
    VkShaderStageFlags stage_flags) {
  bindings_.emplace_back(
      init::GetDescriptorSetLayoutBinding(type, stage_flags, binding));
  writes_.emplace_back(init::GetBufferWriteDescriptorSet(
      type, VK_NULL_HANDLE, &buffer_info, binding));
  return *this;
}

VulkanDescriptorBuilder& VulkanDescriptorBuilder::Bind(
    u32 binding, VkDescriptorImageInfo& image_info, VkDescriptorType type,
    VkShaderStageFlags stage_flags) {
  bindings_.emplace_back(
      init::GetDescriptorSetLayoutBinding(type, stage_flags, binding));
  writes_.emplace_back(init::GetImageWriteDescriptorSet(type, VK_NULL_HANDLE,
                                                        &image_info, binding));
  return *this;
}

void VulkanDescriptorBuilder::Build(VkDescriptorSet& set,
                                    VkDescriptorSetLayout& layout) {
  VkDescriptorSetLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  info.pBindings = bindings_.data();
  info.bindingCount = static_cast<u32>(bindings_.size());
  info.pNext = VK_NULL_HANDLE;
  layout = handler_->Get(info);

  COMET_ASSERT(allocator_->Allocate(set, layout),
               "Could not allocate descriptor set!");

  for (auto& write : writes_) {
    write.dstSet = set;
  }

  vkUpdateDescriptorSets(allocator_->GetDevice(),
                         static_cast<u32>(writes_.size()), writes_.data(), 0,
                         VK_NULL_HANDLE);
}

void VulkanDescriptorBuilder::Build(VkDescriptorSet& set) {
  VkDescriptorSetLayout layout;
  return Build(set, layout);
}

std::size_t GetVulkanDescriptorSetLayoutBindingHash(
    const VkDescriptorSetLayoutBinding& binding) {
  return utils::hash::HashCombine(
      static_cast<std::size_t>(binding.binding) << 32 | binding.descriptorType,
      static_cast<std::size_t>(binding.descriptorCount) << 32 |
          binding.stageFlags);
}

std::size_t GetVulkanDescriptorSetLayoutCreateInfoHash(
    const VkDescriptorSetLayoutCreateInfo& info) {
  auto hash{std::hash<std::size_t>()(info.bindingCount)};
  hash = utils::hash::HashCombine(hash, info.flags);

  for (u32 i{0}; i < info.bindingCount; ++i) {
    hash = utils::hash::HashCombine(
        hash, GetVulkanDescriptorSetLayoutBindingHash(info.pBindings[i]));
  }

  return hash;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet