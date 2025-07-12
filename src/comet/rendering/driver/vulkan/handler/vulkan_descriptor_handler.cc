// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_descriptor_handler.h"

#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_descriptor_utils.h"

namespace comet {
namespace rendering {
namespace vk {
DescriptorHandler::DescriptorHandler(const DescriptorHandlerDescr& descr)
    : Handler(descr) {}

void DescriptorHandler::Initialize() {
  Handler::Initialize();
  auto& device{context_->GetDevice()};

  constexpr StaticArray<VkDescriptorPoolSize, 3> kStaticPoolSizes{
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                           kMaxStaticSetCount_},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                           kMaxStaticSetCount_},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                           kMaxStaticSetCount_}};

  static_descriptor_pool_ = GenerateDescriptorPool(
      device, kMaxStaticSetCount_, kStaticPoolSizes.GetData(),
      static_cast<u32>(kStaticPoolSizes.GetSize()), 0);

  constexpr StaticArray<VkDescriptorPoolSize, 3> kDynamicPoolSizes{
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                           kMaxDynamicSetCount_},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                           kMaxDynamicSetCount_},
      VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                           kMaxDynamicSetCount_}};

  dynamic_descriptor_pools_ = Array<VkDescriptorPool>{&allocator_};
  dynamic_descriptor_pools_.Resize(context_->GetMaxFramesInFlight());

  for (usize i{0}; i < dynamic_descriptor_pools_.GetSize(); ++i) {
    dynamic_descriptor_pools_[i] = GenerateDescriptorPool(
        device, kMaxDynamicSetCount_, kDynamicPoolSizes.GetData(),
        static_cast<u32>(kDynamicPoolSizes.GetSize()),
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
  }
}

void DescriptorHandler::Shutdown() {
  auto& device{context_->GetDevice()};
  DestroyDescriptorPool(device, static_descriptor_pool_);

  for (usize i{0}; i < dynamic_descriptor_pools_.GetSize(); ++i) {
    DestroyDescriptorPool(device, dynamic_descriptor_pools_[i]);
  }

  dynamic_descriptor_pools_.Destroy();
  Handler::Shutdown();
}

bool DescriptorHandler::Generate(
    const VkDescriptorSetLayout* descriptor_set_layout_handles,
    VkDescriptorSet* outDescriptorSets, u32 count, DescriptorType type) {
  auto& device{context_->GetDevice()};
  auto pool{(type == DescriptorType::Static)
                ? static_descriptor_pool_
                : dynamic_descriptor_pools_[context_->GetFrameInFlightIndex()]};
  return AllocateDescriptor(device, descriptor_set_layout_handles,
                            outDescriptorSets, pool, count);
}

bool DescriptorHandler::Generate(
    const Array<VkDescriptorSetLayout>& descriptor_set_layout_handles,
    Array<VkDescriptorSet>& outDescriptorSets, DescriptorType type) {
  auto& device{context_->GetDevice()};
  auto pool{(type == DescriptorType::Static)
                ? static_descriptor_pool_
                : dynamic_descriptor_pools_[context_->GetFrameInFlightIndex()]};
  return AllocateDescriptor(device, descriptor_set_layout_handles,
                            outDescriptorSets, pool);
}

bool DescriptorHandler::Generate(
    const VkDescriptorSetLayout* descriptor_set_layout_handle,
    VkDescriptorSet& outDescriptorSet, DescriptorType type) {
  auto& device{context_->GetDevice()};
  auto pool{(type == DescriptorType::Static)
                ? static_descriptor_pool_
                : dynamic_descriptor_pools_[context_->GetFrameInFlightIndex()]};
  return AllocateDescriptor(device, *descriptor_set_layout_handle,
                            outDescriptorSet, pool);
}

void DescriptorHandler::Destroy(VkDescriptorSet* descriptor_set_handles,
                                u32 count, DescriptorType type) {
  auto& device{context_->GetDevice()};
  auto pool{(type == DescriptorType::Static)
                ? static_descriptor_pool_
                : dynamic_descriptor_pools_[context_->GetFrameInFlightIndex()]};
  FreeDescriptor(device, descriptor_set_handles, pool, count);
  for (u32 i = 0; i < count; ++i) {
    descriptor_set_handles[i] = VK_NULL_HANDLE;
  }
}

void DescriptorHandler::Destroy(Array<VkDescriptorSet>& descriptor_set_handles,
                                DescriptorType type) {
  auto& device{context_->GetDevice()};
  auto pool{(type == DescriptorType::Static)
                ? static_descriptor_pool_
                : dynamic_descriptor_pools_[context_->GetFrameInFlightIndex()]};
  FreeDescriptor(device, descriptor_set_handles, pool);
  for (auto& set : descriptor_set_handles) {
    set = VK_NULL_HANDLE;
  }
}

void DescriptorHandler::Destroy(VkDescriptorSet descriptor_set_handle,
                                DescriptorType type) {
  auto& device{context_->GetDevice()};
  auto pool{(type == DescriptorType::Static)
                ? static_descriptor_pool_
                : dynamic_descriptor_pools_[context_->GetFrameInFlightIndex()]};
  FreeDescriptor(device, descriptor_set_handle, pool);
}

void DescriptorHandler::ResetDynamic() {
  auto dynamic_descriptor_pool{
      dynamic_descriptor_pools_[context_->GetFrameInFlightIndex()]};

  COMET_ASSERT(dynamic_descriptor_pool != VK_NULL_HANDLE,
               "Dynamic descriptor pool is null!");
  vkResetDescriptorPool(context_->GetDevice(), dynamic_descriptor_pool, 0);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet