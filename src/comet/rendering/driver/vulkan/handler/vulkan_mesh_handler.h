// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MESH_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MESH_HANDLER_H_

#include "comet_precompile.h"

#include "comet/rendering/driver/vulkan/data/vulkan_mesh.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/resource/model_resource.h"

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

  void Shutdown() override;

  Mesh* Generate(const resource::MeshResource* resource);
  Mesh* Get(MeshId mesh_id);
  Mesh* Get(const resource::MeshResource* resource);
  Mesh* TryGet(MeshId mesh_id);
  Mesh* TryGet(const resource::MeshResource* resource);
  Mesh* GetOrGenerate(const resource::MeshResource* resource);
  void Destroy(MeshId mesh_id);
  void Destroy(Mesh& mesh);
  void Upload(Mesh& mesh) const;
  MeshId GenerateMeshId(const resource::MeshResource* resource) const;

 private:
  void Destroy(Mesh& mesh, bool is_destroying_handler);

  std::unordered_map<MeshId, Mesh> meshes_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MESH_HANDLER_H_
