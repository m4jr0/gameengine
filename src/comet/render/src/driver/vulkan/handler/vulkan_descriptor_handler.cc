// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/handler/vulkan_descriptor_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/container/array.h"
#include "comet/runtime/profiler/profiler.h"
#include "comet/render/driver/vulkan/utils/vulkan_descriptor_utils.h"

namespace comet {
namespace render {
namespace vk {
DescriptorHandler::DescriptorHandler(const DescriptorHandlerDescr& descr)
    : Handler(descr) {}

bool DescriptorHandler::Generate(
    const VkDescriptorSetLayout* descriptor_set_layout_handles,
    VkDescriptorSet* outDescriptorSets, u32 count, DescriptorType type) {
  COMET_ASSERT(descriptor_set_layout_handles != nullptr,
               "DescriptorHandler::Generate",
               "descriptor set layout handles are null");
  COMET_ASSERT(outDescriptorSets != nullptr, "DescriptorHandler::Generate",
               "output descriptor sets are null");
  COMET_ASSERT(count > 0, "DescriptorHandler::Generate",
               "descriptor set count is zero");

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
  COMET_ASSERT(!descriptor_set_layout_handles.IsEmpty(),
               "DescriptorHandler::Generate",
               "descriptor set layouts are empty");

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
  COMET_ASSERT(descriptor_set_layout_handle != nullptr,
               "DescriptorHandler::Generate",
               "descriptor set layout handle is null");

  auto& device{context_->GetDevice()};
  auto pool{(type == DescriptorType::Static)
                ? static_descriptor_pool_
                : dynamic_descriptor_pools_[context_->GetFrameInFlightIndex()]};
  return AllocateDescriptor(device, *descriptor_set_layout_handle,
                            outDescriptorSet, pool);
}

void DescriptorHandler::Destroy(VkDescriptorSet* descriptor_set_handles,
                                u32 count, DescriptorType type) {
  COMET_ASSERT(descriptor_set_handles != nullptr, "DescriptorHandler::Destroy",
               "descriptor set handles are null");
  COMET_ASSERT(count > 0, "DescriptorHandler::Destroy",
               "descriptor set count is zero");

  auto& device{context_->GetDevice()};
  auto pool{(type == DescriptorType::Static)
                ? static_descriptor_pool_
                : dynamic_descriptor_pools_[context_->GetFrameInFlightIndex()]};
  FreeDescriptor(device, descriptor_set_handles, pool, count);

  for (u32 i{0}; i < count; ++i) {
    descriptor_set_handles[i] = VK_NULL_HANDLE;
  }
}

void DescriptorHandler::Destroy(Array<VkDescriptorSet>& descriptor_set_handles,
                                DescriptorType type) {
  COMET_ASSERT(!descriptor_set_handles.IsEmpty(), "DescriptorHandler::Destroy",
               "descriptor set handles are empty");

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
  COMET_ASSERT(descriptor_set_handle != VK_NULL_HANDLE,
               "DescriptorHandler::Destroy",
               "descriptor set handle is invalid");

  auto& device{context_->GetDevice()};
  auto pool{(type == DescriptorType::Static)
                ? static_descriptor_pool_
                : dynamic_descriptor_pools_[context_->GetFrameInFlightIndex()]};
  FreeDescriptor(device, descriptor_set_handle, pool);
}

void DescriptorHandler::ResetDynamic() {
  COMET_PROFILE("DescriptorHandler::ResetDynamic");

  const auto dynamic_descriptor_pool{
      dynamic_descriptor_pools_[context_->GetFrameInFlightIndex()]};
  COMET_ASSERT(dynamic_descriptor_pool != VK_NULL_HANDLE,
               "DescriptorHandler::ResetDynamic",
               "dynamic descriptor pool is null");

  vkResetDescriptorPool(context_->GetDevice(), dynamic_descriptor_pool, 0);
}

void DescriptorHandler::OnInitialize() {
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

void DescriptorHandler::OnShutdown() {
  auto& device{context_->GetDevice()};
  DestroyDescriptorPool(device, static_descriptor_pool_);

  for (usize i{0}; i < dynamic_descriptor_pools_.GetSize(); ++i) {
    DestroyDescriptorPool(device, dynamic_descriptor_pools_[i]);
  }

  dynamic_descriptor_pools_.Release();
}
}  // namespace vk
}  // namespace render
}  // namespace comet