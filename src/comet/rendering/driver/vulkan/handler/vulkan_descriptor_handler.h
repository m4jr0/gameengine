// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_DESCRIPTOR_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_DESCRIPTOR_HANDLER_H_

#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/data/vulkan_descriptor.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"

namespace comet {
namespace rendering {
namespace vk {
struct DescriptorHandlerDescr : public HandlerDescr {};

class DescriptorHandler : public Handler {
 public:
  DescriptorHandler() = delete;
  explicit DescriptorHandler(const DescriptorHandlerDescr& descr);
  DescriptorHandler(const DescriptorHandler&) = delete;
  DescriptorHandler(DescriptorHandler&&) = delete;
  DescriptorHandler& operator=(const DescriptorHandler&) = delete;
  DescriptorHandler& operator=(DescriptorHandler&&) = delete;
  virtual ~DescriptorHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  bool Generate(const VkDescriptorSetLayout* descriptor_set_layout_handles,
                VkDescriptorSet* outDescriptorSets, u32 count,
                DescriptorType type);

  bool Generate(
      const Array<VkDescriptorSetLayout>& descriptor_set_layout_handles,
      Array<VkDescriptorSet>& outDescriptorSets, DescriptorType type);

  bool Generate(const VkDescriptorSetLayout* descriptor_set_layout_handle,
                VkDescriptorSet& outDescriptorSet, DescriptorType type);

  void Destroy(VkDescriptorSet* descriptor_set_handles, u32 count,
               DescriptorType type);

  void Destroy(Array<VkDescriptorSet>& descriptor_set_handles,
               DescriptorType type);

  void Destroy(VkDescriptorSet descriptor_set_handle, DescriptorType type);

  void ResetDynamic();

 private:
  static inline constexpr u32 kMaxStaticSetCount_{256};
  static inline constexpr u32 kMaxDynamicSetCount_{1024};

  VkDescriptorPool static_descriptor_pool_{VK_NULL_HANDLE};
  Array<VkDescriptorPool> dynamic_descriptor_pools_{};
  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagRendering};
};

}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_DESCRIPTOR_HANDLER_H_
