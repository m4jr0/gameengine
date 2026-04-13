#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PASS_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PASS_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/gid.h"
#include "comet/core/type/map.h"
#include "comet/rendering/driver/vulkan/data/vulkan_frame.h"
#include "comet/rendering/driver/vulkan/data/vulkan_render_pass.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/vulkan_swapchain.h"

namespace comet {
namespace rendering {
namespace vk {
struct RenderPassHandlerDescr : HandlerDescr {
  const Swapchain* swapchain{nullptr};
};

class RenderPassHandler : public Handler {
 public:
  explicit RenderPassHandler(const RenderPassHandlerDescr& descr);
  ~RenderPassHandler() override = default;

  void Initialize() override;
  void Shutdown() override;

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

 private:
  RenderPassHandle Generate(const RenderPassDescr& descr);

  RenderPass& Get(RenderPassHandle handle);
  const RenderPass& Get(RenderPassHandle handle) const;

  void Destroy(RenderPass& render_pass, bool destroying_handler);

  void GenerateFrameBuffers(RenderPass& render_pass) const;
  void DestroyFrameBuffers(RenderPass& render_pass) const;
  void Refresh(RenderPass& render_pass);

  static HashValue GenerateHash(const RenderPassDescr& descr);

 private:
  memory::FiberFreeListAllocator allocator_{sizeof(RenderPass), 32,
                                            memory::kEngineMemoryTagRendering};

  gid::BreedHandler handle_handler_{};

  Array<RenderPass> render_passes_{};
  Array<u32> ref_counts_{};
  Array<HashValue> hashes_{};
  Map<HashValue, RenderPassHandle> cache_{};

  const Swapchain* swapchain_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PASS_HANDLER_H_