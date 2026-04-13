// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_SHADOW_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_SHADOW_VIEW_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shadow.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_lighting_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_mesh_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_proxy_handler.h"
#include "comet/rendering/driver/vulkan/view/vulkan_shader_view.h"

namespace comet {
namespace rendering {
namespace vk {
struct ShadowViewDescr : ShaderViewDescr {
  RenderProxyHandler* render_proxy_handler{nullptr};
  LightingHandler* lighting_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
};

class ShadowView : public ShaderView {
 public:
  ShadowView() = delete;
  explicit ShadowView(const ShadowViewDescr& descr);
  ShadowView(const ShadowView&) = delete;
  ShadowView(ShadowView&&) = delete;
  ShadowView& operator=(const ShadowView&) = delete;
  ShadowView& operator=(ShadowView&&) = delete;
  ~ShadowView() override = default;

  void Initialize() override;
  void Destroy() override;
  void Update(frame::FramePacket*) override;

 private:
  void UpdateShadowShaderPassData();
  void PushShadowConstants(const ShadowRenderJob& job);
  void DrawShadowCasters();
  void SetViewportAndScissor(VkExtent2D extent) const;

  void TransitionShadowMapForRendering(VkCommandBuffer command_buffer_handle,
                                       const ShadowResource& resource,
                                       u32 view_proj_index) const;
  void TransitionShadowMapForSampling(VkCommandBuffer command_buffer_handle,
                                      const ShadowResource& resource,
                                      u32 view_proj_index) const;

  void TransitionShadowLayer(VkCommandBuffer command_buffer_handle,
                             const ShadowResource& resource,
                             u32 view_proj_index, VkImageLayout old_layout,
                             VkImageLayout new_layout,
                             VkAccessFlags src_access_mask,
                             VkAccessFlags dst_access_mask,
                             VkPipelineStageFlags src_stage_mask,
                             VkPipelineStageFlags dst_stage_mask) const;

  RenderProxyHandler* render_proxy_handler_{nullptr};
  LightingHandler* lighting_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};

  Shader* shadow_shader_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_SHADOW_VIEW_H_