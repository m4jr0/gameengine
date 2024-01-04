// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MESH_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MESH_HANDLER_H_

#include "comet_precompile.h"

#include "comet/geometry/geometry_common.h"
#include "comet/rendering/driver/vulkan/data/vulkan_mesh.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"

namespace comet {
namespace rendering {
namespace vk {
using MeshHandlerDescr = HandlerDescr;

class MeshHandler : public Handler {
 public:
  MeshHandler() = delete;
  explicit MeshHandler(const MeshHandlerDescr& descr);
  MeshHandler(const MeshHandler&) = delete;
  MeshHandler(MeshHandler&&) = delete;
  MeshHandler& operator=(const MeshHandler&) = delete;
  MeshHandler& operator=(MeshHandler&&) = delete;
  virtual ~MeshHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  MeshProxy* Generate(geometry::Mesh* geometry);
  MeshProxy* Get(geometry::MeshId proxy_id);
  MeshProxy* Get(const geometry::Mesh* geometry);
  MeshProxy* TryGet(geometry::MeshId proxy_id);
  MeshProxy* TryGet(const geometry::Mesh* geometry);
  MeshProxy* GetOrGenerate(geometry::Mesh* geometry);
  void Destroy(geometry::MeshId proxy_id);
  void Destroy(MeshProxy& proxy);

  void Update(geometry::MeshId proxy_id);
  void Update(MeshProxy& proxy);
  void Bind(geometry::MeshId proxy_id);
  void Bind(const MeshProxy* proxy);

 private:
  void InitializeStagingBuffer();
  void DestroyStagingBuffer();

  MeshProxy* Register(MeshProxy& proxy);
  void Destroy(MeshProxy& proxy, bool is_destroying_handler);
  void Upload(MeshProxy& proxy);

  constexpr static auto kStagingBufferSize{4194304};  // 4 MiB.

  Buffer staging_buffer_{};
  std::unordered_map<geometry::MeshId, MeshProxy> mesh_proxies_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MESH_HANDLER_H_
