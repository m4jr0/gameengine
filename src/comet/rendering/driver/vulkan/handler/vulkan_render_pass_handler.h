// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PASS_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PASS_HANDLER_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/data/vulkan_frame.h"
#include "comet/rendering/driver/vulkan/data/vulkan_render_pass.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_swapchain.h"

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
  RenderPassHandler(RenderPassHandler&& other) = delete;
  RenderPassHandler& operator=(const RenderPassHandler&) = delete;
  RenderPassHandler& operator=(RenderPassHandler&& other) = delete;
  virtual ~RenderPassHandler() = default;

  void Shutdown() override;

  RenderPass* Generate(const RenderPassDescr& descr);
  RenderPass* Get(RenderPassId render_pass_id);
  RenderPass* TryGet(RenderPassId render_pass_id);
  RenderPass* GetOrGenerate(const RenderPassDescr& descr);
  void Destroy(RenderPassId render_pass_id);
  void Destroy(RenderPass& render_pass);
  void BeginPass(const RenderPass& pass, VkCommandBuffer command_buffer_handle,
                 FrameInFlightIndex frame_in_flight_index) const;
  void BeginPass(RenderPassId render_pass_id,
                 VkCommandBuffer command_buffer_handle,
                 FrameInFlightIndex frame_in_flight_index) const;
  void EndPass(VkCommandBuffer command_buffer_handle) const;
  void Refresh(RenderPassId render_pass_id);
  void Refresh(RenderPass& render_pass);

 private:
  void Destroy(RenderPass& render_pass, bool is_destroying_handler);
  void GenerateFrameBuffers(RenderPass& render_pass) const;
  void DestroyFrameBuffers(RenderPass& render_pass) const;

  std::unordered_map<RenderPassId, RenderPass> render_passes_{};
  const Swapchain* swapchain_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PASS_HANDLER_H_
