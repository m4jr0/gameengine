// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GEOMETRY_GEOMETRY_MANAGER_H_
#define COMET_COMET_GEOMETRY_GEOMETRY_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/manager.h"
#include "comet/geometry/component/mesh_component.h"
#include "comet/resource/model_resource.h"

namespace comet {
namespace geometry {
struct SkeletalComponents {
  MeshComponent mesh_cmp{};
  SkeletonComponent skeleton_cmp{};
};

class GeometryManager : public Manager {
 public:
  static GeometryManager& Get();

  GeometryManager() = default;
  GeometryManager(const GeometryManager&) = delete;
  GeometryManager(GeometryManager&&) = delete;
  GeometryManager& operator=(const GeometryManager&) = delete;
  GeometryManager& operator=(GeometryManager&&) = delete;
  virtual ~GeometryManager() = default;

  void Shutdown() override;

  Mesh* Generate(const resource::StaticMeshResource* resource);
  Mesh* Generate(const resource::SkinnedMeshResource* resource);
  Mesh* Get(MeshId mesh_id, geometry::MeshType type);
  Mesh* Get(const resource::MeshResource* resource);
  Mesh* TryGet(MeshId mesh_id, geometry::MeshType type);
  Mesh* TryGet(const resource::MeshResource* resource);
  Mesh* GetOrGenerate(const resource::MeshResource* resource);
  void Destroy(MeshId mesh_id, geometry::MeshType type);
  void Destroy(Mesh& mesh);
  MeshId GenerateMeshId(const resource::MeshResource* resource) const;
  MeshComponent GenerateComponent(const resource::StaticMeshResource* resource);
  SkeletalComponents GenerateComponents(
      const resource::SkinnedMeshResource* resource);

 private:
  Mesh* GenerateInternal(const resource::MeshResource* resource);
  void Destroy(Mesh& mesh, bool is_destroying_handler);

  std::unordered_map<MeshId, Mesh> meshes_{};
};
}  // namespace geometry
}  // namespace comet

#endif  // COMET_COMET_GEOMETRY_GEOMETRY_MANAGER_H_
