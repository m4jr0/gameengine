// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_VIEW_VULKAN_WORLD_VIEW_H_
#define COMET_RENDER_DRIVER_VULKAN_VIEW_VULKAN_WORLD_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/render/driver/vulkan/handler/vulkan_camera_handler.h"
#include "comet/render/driver/vulkan/handler/vulkan_lighting_handler.h"
#include "comet/render/driver/vulkan/handler/vulkan_mesh_handler.h"
#include "comet/render/driver/vulkan/handler/vulkan_render_proxy_handler.h"
#include "comet/render/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/render/driver/vulkan/handler/vulkan_texture_handler.h"
#include "comet/render/driver/vulkan/view/vulkan_view.h"
#include "comet/render/render_handle.h"
#include "comet/render/type/light.h"

namespace comet {
namespace rendering {
namespace vk {
struct WorldViewDescr : ViewDescr {
  const ShadowSettings* shadow_settings{nullptr};
  CameraHandler* camera_handler{nullptr};
  ShaderHandler* shader_handler{nullptr};
  const TextureHandler* texture_handler{nullptr};
  RenderProxyHandler* render_proxy_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
  LightingHandler* lighting_handler{nullptr};
};

class WorldView : public View {
 public:
  explicit WorldView(const WorldViewDescr& descr);
  WorldView(const WorldView&) = delete;
  WorldView(WorldView&&) = delete;
  WorldView& operator=(const WorldView&) = delete;
  WorldView& operator=(WorldView&&) = delete;
  ~WorldView() override = default;

  void Prepare(const ViewUpdate& update) override;
  void Begin(const ViewUpdate&) override;
  void Draw(const ViewUpdate& update) override;
  void End(const ViewUpdate& update) override;

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  void UpdateWorldPassShaderData(const frame::FramePacket* packet);
  void DrawWorld(const ViewUpdate& update);

  void SetViewportAndScissor(const ViewportRect& viewport) const;

  const ShadowSettings* shadow_settings_{nullptr};

  CameraHandler* camera_handler_{nullptr};
  ShaderHandler* shader_handler_{nullptr};
  const TextureHandler* texture_handler_{nullptr};
  RenderProxyHandler* render_proxy_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  LightingHandler* lighting_handler_{nullptr};

  ShaderHandle world_shader_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_VIEW_VULKAN_WORLD_VIEW_H_