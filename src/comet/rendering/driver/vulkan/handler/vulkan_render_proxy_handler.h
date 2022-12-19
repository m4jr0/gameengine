// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_PROXY_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_PROXY_HANDLER_H_

#include "comet_precompile.h"

#include "glm/glm.hpp"

#include "comet/entity/entity_manager.h"
#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/data/vulkan_mesh.h"
#include "comet/rendering/driver/vulkan/data/vulkan_proxy.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_material_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_mesh_handler.h"
#include "comet/resource/model_resource.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace rendering {
namespace vk {
struct RenderProxyHandlerDescr : HandlerDescr {
  MaterialHandler* material_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
};

class RenderProxyHandler : public Handler {
 public:
  RenderProxyHandler() = delete;
  explicit RenderProxyHandler(const RenderProxyHandlerDescr& descr);
  RenderProxyHandler(const RenderProxyHandler&) = delete;
  RenderProxyHandler(RenderProxyHandler&& other) = delete;
  RenderProxyHandler& operator=(const RenderProxyHandler&) = delete;
  RenderProxyHandler& operator=(RenderProxyHandler&& other) = delete;
  virtual ~RenderProxyHandler() = default;

  void Shutdown() override;

  RenderProxy* Get(uindex index);
  RenderProxy* Get(const resource::MeshResource* resource);
  RenderProxy* TryGet(uindex index);
  RenderProxy* TryGet(const resource::MeshResource* resource);
  void Update(time::Interpolation interpolation);
  void DrawProxies(const Shader& shader);
  uindex GetCount() const noexcept;

 private:
  RenderProxy GenerateInternal(Mesh& mesh, Material& material,
                               const glm::mat4& transform);
  void Draw(const RenderProxy& proxy);

  FrameIndex update_frame_{kInvalidFrameIndex};
  std::vector<RenderProxy> proxies_{};
  const RenderProxy* last_drawn_proxy_{nullptr};
  MaterialHandler* material_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_PROXY_HANDLER_H_
