// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_WORLD_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_WORLD_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_lighting_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_mesh_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_proxy_handler.h"
#include "comet/rendering/driver/vulkan/view/vulkan_shader_view.h"
#include "comet/rendering/light/light_common.h"

namespace comet {
namespace rendering {
namespace vk {
struct WorldViewDescr : ShaderViewDescr {
  const ShadowSettings* shadow_settings{nullptr};
  RenderProxyHandler* render_proxy_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
  LightingHandler* lighting_handler{nullptr};
};

class WorldView : public ShaderView {
 public:
  explicit WorldView(const WorldViewDescr& descr);
  WorldView(const WorldView&) = delete;
  WorldView(WorldView&&) = delete;
  WorldView& operator=(const WorldView&) = delete;
  WorldView& operator=(WorldView&&) = delete;
  virtual ~WorldView() = default;

  void Initialize() override;
  void Destroy() override;
  void Update(frame::FramePacket* packet) override;

 private:
  void UpdateWorldShader(frame::FramePacket* packet);
  void RunSparseUpload();
  void RunCull(frame::FramePacket* packet);
  void DrawWorld();
  void SetViewportAndScissor() const;

  const ShadowSettings* shadow_settings_{nullptr};

  RenderProxyHandler* render_proxy_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  LightingHandler* lighting_handler_{nullptr};

  Shader* world_shader_{nullptr};
  Shader* cull_shader_{nullptr};
  Shader* sparse_upload_shader_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VIEW_VULKAN_WORLD_VIEW_H_