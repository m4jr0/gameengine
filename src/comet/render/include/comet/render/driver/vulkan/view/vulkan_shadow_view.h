// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_VIEW_VULKAN_SHADOW_VIEW_H_
#define COMET_RENDER_DRIVER_VULKAN_VIEW_VULKAN_SHADOW_VIEW_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_lighting_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_mesh_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_proxy_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_shadow.h"
#include "comet/rendering/driver/vulkan/view/vulkan_view.h"
#include "comet/rendering/rendering_handle.h"

namespace comet {
namespace rendering {
namespace vk {
struct ShadowViewDescr : ViewDescr {
  ShaderHandler* shader_handler{nullptr};
  RenderProxyHandler* render_proxy_handler{nullptr};
  LightingHandler* lighting_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
};

class ShadowView : public View {
 public:
  ShadowView() = delete;
  explicit ShadowView(const ShadowViewDescr& descr);
  ShadowView(const ShadowView&) = delete;
  ShadowView(ShadowView&&) = delete;
  ShadowView& operator=(const ShadowView&) = delete;
  ShadowView& operator=(ShadowView&&) = delete;
  ~ShadowView() override = default;

  void Prepare(const ViewUpdate&) override;
  void Begin(const ViewUpdate& update) override;
  void Draw(const ViewUpdate& update) override;
  void End(const ViewUpdate& update) override;

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  void UpdateShadowShaderPassData();
  void PushShadowConstants(const ShadowRenderJob& job);
  void DrawShadowCasters(u32 shadow_job_index);

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

  ShaderHandler* shader_handler_{nullptr};
  RenderProxyHandler* render_proxy_handler_{nullptr};
  LightingHandler* lighting_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};

  ShaderHandle shadow_shader_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_VIEW_VULKAN_SHADOW_VIEW_H_