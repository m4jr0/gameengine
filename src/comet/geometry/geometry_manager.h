// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GEOMETRY_GEOMETRY_MANAGER_H_
#define COMET_COMET_GEOMETRY_GEOMETRY_MANAGER_H_

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/map.h"
#include "comet/entity/entity_id.h"
#include "comet/geometry/component/mesh_component.h"
#include "comet/geometry/component/model_component.h"
#include "comet/geometry/component/skeleton_component.h"
#include "comet/geometry/type/geometry_mesh_type.h"
#include "comet/resource/model/model_resource.h"
#include "comet/resource/type/resource_common_type.h"

namespace comet {
namespace geometry {
class GeometryManager : public Manager {
 public:
  static GeometryManager& Get();

  GeometryManager();
  ~GeometryManager() override = default;

  MeshHandle GetOrGenerate(const resource::MeshResource& resource);

  MeshHandle GetOrGenerate(MeshId mesh_id, MeshType type,
                           const Array<SkinnedVertex>& vertices,
                           const Array<Index>& indices,
                           const math::Vec3& local_center,
                           const math::Vec3& local_max_extents);

  MeshHandle GenerateCube(f32 size = 1.0f);

  void Destroy(MeshHandle handle);
  void Destroy(MeshId mesh_id);

  const Mesh* Get(MeshHandle handle) const;
  const Mesh* TryGet(MeshHandle handle) const;

  MeshHandle ResolveHandle(MeshId mesh_id) const;
  MeshHandle ResolveHandle(const resource::MeshResource& resource) const;

  MeshHandle TryResolveHandle(MeshId mesh_id) const;
  MeshHandle TryResolveHandle(const resource::MeshResource& resource) const;

  MeshId GenerateMeshId(const resource::MeshResource& resource) const;

  void PopulateGeometryData(MeshHandle handle, frame::AddedGeometry& out) const;
  void PopulateGeometryData(MeshHandle handle, frame::DirtyMesh& out) const;

  StaticModelComponent GenerateStaticModelComponent(
      entity::EntityId entity_id, CTStringView model_path,
      resource::ResourceLifeSpan life_span =
          resource::ResourceLifeSpan::Manual) const;
  SkeletalModelComponent GenerateSkeletalModelComponent(
      entity::EntityId entity_id, CTStringView model_path,
      resource::ResourceLifeSpan life_span =
          resource::ResourceLifeSpan::Manual) const;
  MeshComponent GenerateStaticMeshComponent(
      const resource::StaticMeshResource& resource, entity::EntityId entity_id,
      entity::EntityId model_entity_id);
  MeshComponent GenerateSkinnedMeshComponent(
      const resource::SkinnedMeshResource& resource, entity::EntityId entity_id,
      entity::EntityId model_entity_id);
  SkeletonComponent GenerateSkeletonComponent(
      CTStringView model_path, resource::ResourceLifeSpan life_span =
                                   resource::ResourceLifeSpan::Manual) const;

  void DestroyStaticModelComponent(StaticModelComponent* model_cmp) const;
  void DestroySkeletalModelComponent(SkeletalModelComponent* model_cmp) const;
  void DestroyStaticMeshComponent(MeshComponent* mesh_cmp);
  void DestroySkinnedMeshComponent(MeshComponent* mesh_cmp);
  void DestroySkeletonComponent(SkeletonComponent* skeleton_cmp) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  Mesh* Get(MeshHandle handle);
  Mesh* TryGet(MeshHandle handle);

  Mesh* CreateMeshObject(const resource::MeshResource& resource,
                         MeshHandle handle);

  Mesh* CreateProceduralMeshObject(MeshId mesh_id, MeshType type,
                                   const Array<SkinnedVertex>& vertices,
                                   const Array<Index>& indices,
                                   const math::Vec3& local_center,
                                   const math::Vec3& local_max_extents,
                                   MeshHandle handle);

  void DestroyMeshObject(Mesh* mesh);
  void Destroy(MeshHandle handle, bool is_shutdown);

 private:
  memory::FiberFreeListAllocator mesh_allocator_{};
  memory::FiberFreeListAllocator mesh_array_allocator_{};
  memory::FiberFreeListAllocator mesh_map_allocator_{};
  memory::FiberFreeListAllocator vertex_allocator_{};
  memory::FiberFreeListAllocator index_allocator_{};

  mutable fiber::FiberMutex mutex_{};

  HandlePool<MeshTag> mesh_pool_{};
  Array<Mesh*> meshes_{};
  Map<MeshId, MeshHandle> mesh_handles_by_id_{};
};
}  // namespace geometry
}  // namespace comet

#endif  // COMET_COMET_GEOMETRY_GEOMETRY_MANAGER_H_