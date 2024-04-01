// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MESH_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MESH_HANDLER_H_

#include "glad/glad.h"

#include "comet/core/essentials.h"
#include "comet/geometry/geometry_common.h"
#include "comet/rendering/driver/opengl/data/opengl_mesh.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"

namespace comet {
namespace rendering {
namespace gl {
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

  MeshProxy* Generate(const geometry::Mesh* resource);
  MeshProxy* Get(geometry::MeshId proxy_id);
  MeshProxy* Get(const geometry::Mesh* mesh);
  MeshProxy* TryGet(geometry::MeshId proxy_id);
  MeshProxy* TryGet(const geometry::Mesh* mesh);
  MeshProxy* GetOrGenerate(const geometry::Mesh* mesh);
  void Destroy(geometry::MeshId proxy_id);
  void Destroy(MeshProxy& proxy);

  void Update(geometry::MeshId proxy_id);
  void Update(MeshProxy& proxy);
  void Bind(geometry::MeshId proxy_id);
  void Bind(const MeshProxy* proxy);

 private:
  std::unordered_map<geometry::MeshId, MeshProxy> mesh_proxies_{};

  MeshProxy* Register(MeshProxy& proxy);
  void Destroy(MeshProxy& proxy, bool is_destroying_handler);
  void Upload(MeshProxy& proxy) const;
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MESH_HANDLER_H_
