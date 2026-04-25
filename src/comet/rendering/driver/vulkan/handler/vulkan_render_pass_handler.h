// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PASS_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PASS_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/shared_instance_registry.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_frame.h"
#include "comet/rendering/driver/vulkan/type/vulkan_render_pass.h"
#include "comet/rendering/driver/vulkan/vulkan_swapchain.h"
#include "comet/rendering/rendering_handle.h"

namespace comet {
namespace rendering {
namespace vk {
struct RenderPassHandlerDescr : HandlerDescr {
  const Swapchain* swapchain{nullptr};
};

class RenderPassHandler : public Handler {
 public:
  RenderPassHandler() = delete;
  explicit RenderPassHandler(const RenderPassHandlerDescr& descr);
  RenderPassHandler(const RenderPassHandler&) = delete;
  RenderPassHandler(RenderPassHandler&&) = delete;
  RenderPassHandler& operator=(const RenderPassHandler&) = delete;
  RenderPassHandler& operator=(RenderPassHandler&&) = delete;
  ~RenderPassHandler() override = default;

  RenderPassHandle GetOrGenerate(const RenderPassDescr& descr);

  void Destroy(RenderPassHandle handle);

  void BeginPass(RenderPassHandle handle, VkCommandBuffer cmd,
                 ImageIndex image_index, const VkClearValue* clear_values,
                 u32 clear_value_count) const;
  void BeginPass(RenderPassHandle handle, VkCommandBuffer cmd,
                 VkFramebuffer framebuffer, const VkClearValue* clear_values,
                 u32 clear_value_count) const;

  void EndPass(VkCommandBuffer cmd) const;

  void SetSize(RenderPassHandle handle, u32 width, u32 height);

  void Refresh(RenderPassHandle handle);

  VkRenderPass GetVkHandle(RenderPassHandle handle) const;
  VkSampleCountFlagBits GetSamples(RenderPassHandle handle) const;
  VkExtent2D GetExtent(RenderPassHandle handle) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  RenderPass* GenerateRenderPass(const RenderPassDescr& descr);
  void DestroyRenderPass(RenderPass* render_pass);

  RenderPass* Get(RenderPassHandle handle);
  const RenderPass* Get(RenderPassHandle handle) const;

  void GenerateFrameBuffers(RenderPass& render_pass) const;
  void DestroyFrameBuffers(RenderPass& render_pass) const;

  void Refresh(RenderPass& render_pass);

  static HashValue GenerateHash(const RenderPassDescr& descr);

 private:
  memory::PlatformAllocator cache_allocator_{memory::kEngineMemoryTagRendering};

  memory::FiberFreeListAllocator allocator_{sizeof(RenderPass), 32,
                                            memory::kEngineMemoryTagRendering};

  SharedInstanceRegistry<HashValue, RenderPassHandleTag, RenderPass>
      render_passes_{};

  const Swapchain* swapchain_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PASS_HANDLER_H_