// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_PROXY_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_PROXY_HANDLER_H_

#include "comet_precompile.h"

#include "comet/entity/entity_manager.h"
#include "comet/math/bounding_volume.h"
#include "comet/math/matrix.h"
#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/data/vulkan_mesh.h"
#include "comet/rendering/driver/vulkan/data/vulkan_render_proxy.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_material_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_mesh_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/resource/model_resource.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace rendering {
namespace vk {
struct RenderProxyHandlerDescr : HandlerDescr {
  MaterialHandler* material_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
  ShaderHandler* shader_handler{nullptr};
  CameraManager* camera_manager{nullptr};
  entity::EntityManager* entity_manager{nullptr};
};

class RenderProxyHandler : public Handler {
 public:
  RenderProxyHandler() = delete;
  explicit RenderProxyHandler(const RenderProxyHandlerDescr& descr);
  RenderProxyHandler(const RenderProxyHandler&) = delete;
  RenderProxyHandler(RenderProxyHandler&&) = delete;
  RenderProxyHandler& operator=(const RenderProxyHandler&) = delete;
  RenderProxyHandler& operator=(RenderProxyHandler&&) = delete;
  virtual ~RenderProxyHandler() = default;

  void Shutdown() override;

  void Update(time::Interpolation interpolation);
  void DrawProxies(Shader& shader);
  // TODO(m4jr0): Remove temporary code.
  void DrawProxiesForDebugging(Shader& shader);
  u32 GetDrawCount() const noexcept;

 private:
  RenderProxy GenerateInternal(Mesh& mesh, Material& material,
                               const math::Mat4& transform);
  void Draw(const RenderProxy& proxy);

  constexpr static auto kDefaultProxyCount{512};
  FrameIndex update_frame_{kInvalidFrameIndex};
  std::vector<RenderProxy> proxies_{};
  const Mesh* last_drawn_mesh_{nullptr};
  MaterialHandler* material_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  ShaderHandler* shader_handler_{nullptr};
  CameraManager* camera_manager_{nullptr};
  entity::EntityManager* entity_manager_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_PROXY_HANDLER_H_
