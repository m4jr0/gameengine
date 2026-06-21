// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_DESCRIPTOR_HANDLER_H_
#define COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_DESCRIPTOR_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/memory/memory_tag.h"
#include "comet/core/container/array.h"
#include "comet/render/driver/vulkan/handler/vulkan_handler.h"
#include "comet/render/driver/vulkan/type/vulkan_descriptor.h"

namespace comet {
namespace render {
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
  ~DescriptorHandler() override = default;

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

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  static inline constexpr u32 kMaxStaticSetCount_{256};
  static inline constexpr u32 kMaxDynamicSetCount_{1024};

  VkDescriptorPool static_descriptor_pool_{VK_NULL_HANDLE};
  Array<VkDescriptorPool> dynamic_descriptor_pools_{};
  memory::PlatformAllocator allocator_{kEngineMemoryTagRender};
};

}  // namespace vk
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_DESCRIPTOR_HANDLER_H_
