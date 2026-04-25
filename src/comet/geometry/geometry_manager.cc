// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "geometry_manager.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/geometry/label/mesh_label.h"
#include "comet/geometry/type/skeleton.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace geometry {
namespace internal {
static MeshId GetDebugUnitCubeMeshId() noexcept {
  return static_cast<MeshId>(COMET_STRING_ID("debug_unit_cube_mesh"));
}
}  // namespace internal

GeometryManager& GeometryManager::Get() {
  static GeometryManager singleton{};
  return singleton;
}

GeometryManager::GeometryManager()
    : mesh_allocator_{sizeof(Mesh), 1024, memory::kEngineMemoryTagGeometry},
      mesh_array_allocator_{sizeof(Mesh*), 1024,
                            memory::kEngineMemoryTagGeometry},
      mesh_map_allocator_{sizeof(Pair<MeshId, MeshHandle>), 1024,
                          memory::kEngineMemoryTagGeometry},
      vertex_allocator_{sizeof(SkinnedVertex), 1024,
                        memory::kEngineMemoryTagGeometry},
      index_allocator_{sizeof(Index), 1024, memory::kEngineMemoryTagGeometry} {}

MeshHandle GeometryManager::GetOrGenerate(
    const resource::MeshResource& resource) {
  COMET_PROFILE("GeometryManager::GetOrGenerate");
  fiber::FiberLockGuard lock{mutex_};

  const auto mesh_id{GenerateMeshId(resource)};
  auto* existing_handle{mesh_handles_by_id_.TryGet(mesh_id)};

  if (existing_handle != nullptr && mesh_pool_.IsAlive(*existing_handle)) {
    const auto index{static_cast<usize>(existing_handle->GetIndex())};

    auto* mesh{meshes_[index]};
    COMET_ASSERT(mesh != nullptr, "GeometryManager::GetOrGenerate",
                 "cached mesh is null", "mesh_id", mesh_id, "handle",
                 *existing_handle);

    ++mesh->ref_count;
    return *existing_handle;
  }

  const auto handle{mesh_pool_.Generate()};
  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= meshes_.GetSize()) {
    meshes_.Resize(index + 1);
  }

  auto* mesh{CreateMeshObject(resource, handle)};
  meshes_[index] = mesh;
  mesh_handles_by_id_.Set(mesh->id, handle);

  return handle;
}

MeshHandle GeometryManager::GetOrGenerate(MeshId mesh_id, MeshType type,
                                          const Array<SkinnedVertex>& vertices,
                                          const Array<Index>& indices,
                                          const math::Vec3& local_center,
                                          const math::Vec3& local_max_extents) {
  COMET_PROFILE("GeometryManager::GetOrGenerate");
  fiber::FiberLockGuard lock{mutex_};

  auto* existing_handle{mesh_handles_by_id_.TryGet(mesh_id)};

  if (existing_handle != nullptr && mesh_pool_.IsAlive(*existing_handle)) {
    const auto index{static_cast<usize>(existing_handle->GetIndex())};

    auto* mesh{meshes_[index]};
    COMET_ASSERT(mesh != nullptr, "GeometryManager::GetOrGenerate",
                 "cached mesh is null", "mesh_id", mesh_id, "handle",
                 *existing_handle);

    ++mesh->ref_count;
    return *existing_handle;
  }

  const auto handle{mesh_pool_.Generate()};
  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= meshes_.GetSize()) {
    meshes_.Resize(index + 1);
  }

  auto* mesh{CreateProceduralMeshObject(mesh_id, type, vertices, indices,
                                        local_center, local_max_extents,
                                        handle)};
  meshes_[index] = mesh;
  mesh_handles_by_id_.Set(mesh->id, handle);

  return handle;
}

MeshHandle GeometryManager::GenerateCube(f32 size) {
  COMET_PROFILE("GeometryManager::GenerateCube");
  COMET_ASSERT(size > .0f, "GeometryManager::GenerateCube",
               "cube size must be greater than zero", "size", size);

  const f32 h{size * .5f};

  Array<SkinnedVertex> vertices{&vertex_allocator_};
  Array<Index> indices{&index_allocator_};

  vertices.Reserve(24);
  indices.Reserve(36);

  const auto push_vertex{[&](const math::Vec3& position,
                             const math::Vec3& normal,
                             const math::Vec4& tangent, const math::Vec2& uv) {
    auto& vertex{vertices.EmplaceLast()};
    vertex.position = position;
    vertex.normal = normal;
    vertex.tangent = tangent;
    vertex.uv = uv;
    vertex.color = math::Vec4{1.0f};

    for (u32 i{0}; i < kMaxSkeletonJointCount; ++i) {
      vertex.joint_indices[i] = kInvalidSkeletonJointIndex;
      vertex.joint_weights[i] = .0f;
    }
  }};

  const auto push_face{[&](const math::Vec3& p0, const math::Vec3& p1,
                           const math::Vec3& p2, const math::Vec3& p3,
                           const math::Vec3& normal,
                           const math::Vec4& tangent) {
    const auto base_index{static_cast<Index>(vertices.GetSize())};

    push_vertex(p0, normal, tangent, math::Vec2{.0f, .0f});
    push_vertex(p1, normal, tangent, math::Vec2{1.0f, .0f});
    push_vertex(p2, normal, tangent, math::Vec2{1.0f, 1.0f});
    push_vertex(p3, normal, tangent, math::Vec2{.0f, 1.0f});

    indices.PushLast(base_index + 0);
    indices.PushLast(base_index + 1);
    indices.PushLast(base_index + 2);

    indices.PushLast(base_index + 0);
    indices.PushLast(base_index + 2);
    indices.PushLast(base_index + 3);
  }};

  push_face(math::Vec3{h, -h, -h}, math::Vec3{h, -h, h}, math::Vec3{h, h, h},
            math::Vec3{h, h, -h}, math::Vec3{1.0f, .0f, .0f},
            math::Vec4{.0f, .0f, 1.0f, 1.0f});

  push_face(math::Vec3{-h, -h, h}, math::Vec3{-h, -h, -h},
            math::Vec3{-h, h, -h}, math::Vec3{-h, h, h},
            math::Vec3{-1.0f, .0f, .0f}, math::Vec4{.0f, .0f, -1.0f, 1.0f});

  push_face(math::Vec3{-h, h, -h}, math::Vec3{h, h, -h}, math::Vec3{h, h, h},
            math::Vec3{-h, h, h}, math::Vec3{.0f, 1.0f, .0f},
            math::Vec4{1.0f, .0f, .0f, 1.0f});

  push_face(math::Vec3{-h, -h, h}, math::Vec3{h, -h, h}, math::Vec3{h, -h, -h},
            math::Vec3{-h, -h, -h}, math::Vec3{.0f, -1.0f, .0f},
            math::Vec4{1.0f, .0f, .0f, 1.0f});

  push_face(math::Vec3{-h, -h, h}, math::Vec3{-h, h, h}, math::Vec3{h, h, h},
            math::Vec3{h, -h, h}, math::Vec3{.0f, .0f, 1.0f},
            math::Vec4{1.0f, .0f, .0f, 1.0f});

  push_face(math::Vec3{h, -h, -h}, math::Vec3{h, h, -h}, math::Vec3{-h, h, -h},
            math::Vec3{-h, -h, -h}, math::Vec3{.0f, .0f, -1.0f},
            math::Vec4{-1.0f, .0f, .0f, 1.0f});

  return GetOrGenerate(internal::GetDebugUnitCubeMeshId(), MeshType::Static,
                       vertices, indices, math::Vec3{.0f}, math::Vec3{h, h, h});
}

void GeometryManager::Destroy(MeshHandle handle) { Destroy(handle, false); }

void GeometryManager::Destroy(MeshId mesh_id) {
  const auto handle{ResolveHandle(mesh_id)};
  Destroy(handle);
}

const Mesh* GeometryManager::Get(MeshHandle handle) const {
  const auto* mesh{TryGet(handle)};
  COMET_ASSERT(mesh != nullptr, "GeometryManager::Get", "mesh does not exist",
               "handle", handle);
  return mesh;
}

const Mesh* GeometryManager::TryGet(MeshHandle handle) const {
  fiber::FiberLockGuard lock{mutex_};

  if (!mesh_pool_.IsAlive(handle)) {
    return nullptr;
  }

  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= meshes_.GetSize()) {
    return nullptr;
  }

  return meshes_[index];
}

MeshHandle GeometryManager::ResolveHandle(MeshId mesh_id) const {
  const auto handle{TryResolveHandle(mesh_id)};
  COMET_ASSERT(handle, "GeometryManager::ResolveHandle", "mesh does not exist",
               "mesh_id", mesh_id);
  return handle;
}

MeshHandle GeometryManager::ResolveHandle(
    const resource::MeshResource& resource) const {
  return ResolveHandle(GenerateMeshId(resource));
}

MeshHandle GeometryManager::TryResolveHandle(MeshId mesh_id) const {
  fiber::FiberLockGuard lock{mutex_};

  const auto* handle_ptr{mesh_handles_by_id_.TryGet(mesh_id)};

  if (handle_ptr == nullptr) {
    return MeshHandle::Invalid();
  }

  if (!mesh_pool_.IsAlive(*handle_ptr)) {
    return MeshHandle::Invalid();
  }

  return *handle_ptr;
}

MeshHandle GeometryManager::TryResolveHandle(
    const resource::MeshResource& resource) const {
  return TryResolveHandle(GenerateMeshId(resource));
}

MeshId GeometryManager::GenerateMeshId(
    const resource::MeshResource& resource) const {
  return static_cast<u64>(resource.internal_id) |
         (static_cast<u64>(resource.resource_id) << 32);
}

void GeometryManager::PopulateGeometryData(MeshHandle handle,
                                           frame::AddedGeometry& out) const {
  COMET_PROFILE("GeometryManager::PopulateGeometryData");
  const auto* mesh{Get(handle)};

  out.mesh_handle = mesh->handle;
  out.local_center = mesh->local_center;
  out.local_max_extents = mesh->local_max_extents;

  COMET_ASSERT(out.vertices != nullptr, "GeometryManager::PopulateGeometryData",
               "output vertices are null", "handle", handle);
  COMET_ASSERT(out.indices != nullptr, "GeometryManager::PopulateGeometryData",
               "output indices are null", "handle", handle);

  out.vertices->PushFromRange(mesh->vertices);
  out.indices->PushFromRange(mesh->indices);
}

void GeometryManager::PopulateGeometryData(MeshHandle handle,
                                           frame::DirtyMesh& out) const {
  COMET_PROFILE("GeometryManager::PopulateGeometryData");
  const auto* mesh{Get(handle)};

  out.mesh_handle = mesh->handle;
  out.local_center = mesh->local_center;
  out.local_max_extents = mesh->local_max_extents;

  COMET_ASSERT(out.vertices != nullptr, "GeometryManager::PopulateGeometryData",
               "output vertices are null", "handle", handle);
  COMET_ASSERT(out.indices != nullptr, "GeometryManager::PopulateGeometryData",
               "output indices are null", "handle", handle);

  out.vertices->PushFromRange(mesh->vertices);
  out.indices->PushFromRange(mesh->indices);
}

StaticModelComponent GeometryManager::GenerateStaticModelComponent(
    entity::EntityId entity_id, CTStringView model_path,
    resource::ResourceLifeSpan life_span) const {
  COMET_PROFILE("GeometryManager::GenerateStaticModelComponent");
  StaticModelComponent model_cmp{};
  model_cmp.entity_id = entity_id;
  model_cmp.resource_handle =
      resource::ResourceManager::Get().GetStaticModels()->Load(model_path,
                                                               life_span);
  return model_cmp;
}

SkeletalModelComponent GeometryManager::GenerateSkeletalModelComponent(
    entity::EntityId entity_id, CTStringView model_path,
    resource::ResourceLifeSpan life_span) const {
  COMET_PROFILE("GeometryManager::GenerateSkeletalModelComponent");
  SkeletalModelComponent model_cmp{};
  model_cmp.entity_id = entity_id;
  model_cmp.resource_handle =
      resource::ResourceManager::Get().GetSkeletalModels()->Load(model_path,
                                                                 life_span);
  return model_cmp;
}

MeshComponent GeometryManager::GenerateStaticMeshComponent(
    const resource::StaticMeshResource& resource, entity::EntityId entity_id,
    entity::EntityId model_entity_id) {
  COMET_PROFILE("GeometryManager::GenerateStaticMeshComponent");
  MeshComponent mesh_cmp{};
  mesh_cmp.entity_id = entity_id;
  mesh_cmp.model_entity_id = model_entity_id;
  mesh_cmp.mesh_handle = GetOrGenerate(resource);
  mesh_cmp.material_resource_id = resource.material_resource_id;
  return mesh_cmp;
}

MeshComponent GeometryManager::GenerateSkinnedMeshComponent(
    const resource::SkinnedMeshResource& resource, entity::EntityId entity_id,
    entity::EntityId model_entity_id) {
  COMET_PROFILE("GeometryManager::GenerateSkinnedMeshComponent");
  MeshComponent mesh_cmp{};
  mesh_cmp.entity_id = entity_id;
  mesh_cmp.model_entity_id = model_entity_id;
  mesh_cmp.mesh_handle = GetOrGenerate(resource);
  mesh_cmp.material_resource_id = resource.material_resource_id;
  return mesh_cmp;
}

SkeletonComponent GeometryManager::GenerateSkeletonComponent(
    CTStringView model_path, resource::ResourceLifeSpan life_span) const {
  COMET_PROFILE("GeometryManager::GenerateSkeletonComponent");
  SkeletonComponent skeleton_cmp{};
  skeleton_cmp.resource_handle =
      resource::ResourceManager::Get().GetSkeletons()->Load(model_path,
                                                            life_span);
  return skeleton_cmp;
}

void GeometryManager::DestroyStaticModelComponent(
    StaticModelComponent* model_cmp) const {
  COMET_PROFILE("GeometryManager::DestroyStaticModelComponent");
  COMET_ASSERT(model_cmp != nullptr,
               "GeometryManager::DestroyStaticModelComponent",
               "component is null");

  model_cmp->entity_id = entity::kInvalidEntityId;

  if (model_cmp->resource_handle) {
    resource::ResourceManager::Get().GetStaticModels()->Unload(
        model_cmp->resource_handle);
    model_cmp->resource_handle.Invalidate();
  }
}

void GeometryManager::DestroySkeletalModelComponent(
    SkeletalModelComponent* model_cmp) const {
  COMET_PROFILE("GeometryManager::DestroySkeletalModelComponent");
  COMET_ASSERT(model_cmp != nullptr,
               "GeometryManager::DestroySkeletalModelComponent",
               "component is null");

  model_cmp->entity_id = entity::kInvalidEntityId;

  if (model_cmp->resource_handle) {
    resource::ResourceManager::Get().GetSkeletalModels()->Unload(
        model_cmp->resource_handle);
    model_cmp->resource_handle.Invalidate();
  }
}

void GeometryManager::DestroyStaticMeshComponent(MeshComponent* mesh_cmp) {
  COMET_PROFILE("GeometryManager::DestroyStaticMeshComponent");
  COMET_ASSERT(mesh_cmp != nullptr,
               "GeometryManager::DestroyStaticMeshComponent",
               "component is null");

  mesh_cmp->entity_id = entity::kInvalidEntityId;
  mesh_cmp->model_entity_id = entity::kInvalidEntityId;

  if (mesh_cmp->mesh_handle) {
    Destroy(mesh_cmp->mesh_handle);
    mesh_cmp->mesh_handle.Invalidate();
  }

  mesh_cmp->material_resource_id.Invalidate();
}

void GeometryManager::DestroySkinnedMeshComponent(MeshComponent* mesh_cmp) {
  COMET_PROFILE("GeometryManager::DestroySkinnedMeshComponent");
  COMET_ASSERT(mesh_cmp != nullptr,
               "GeometryManager::DestroySkinnedMeshComponent",
               "component is null");

  mesh_cmp->entity_id = entity::kInvalidEntityId;
  mesh_cmp->model_entity_id = entity::kInvalidEntityId;

  if (mesh_cmp->mesh_handle) {
    Destroy(mesh_cmp->mesh_handle);
    mesh_cmp->mesh_handle.Invalidate();
  }

  mesh_cmp->material_resource_id.Invalidate();
}

void GeometryManager::DestroySkeletonComponent(
    SkeletonComponent* skeleton_cmp) const {
  COMET_PROFILE("GeometryManager::DestroySkeletonComponent");
  COMET_ASSERT(skeleton_cmp != nullptr,
               "GeometryManager::DestroySkeletonComponent",
               "component is null");

  if (skeleton_cmp->resource_handle) {
    resource::ResourceManager::Get().GetSkeletons()->Unload(
        skeleton_cmp->resource_handle);
    skeleton_cmp->resource_handle.Invalidate();
  }
}

void GeometryManager::OnInitialize() {
  mesh_allocator_.Initialize();
  mesh_array_allocator_.Initialize();
  mesh_map_allocator_.Initialize();
  vertex_allocator_.Initialize();
  index_allocator_.Initialize();

  meshes_ = Array<Mesh*>{&mesh_array_allocator_};
  mesh_handles_by_id_ = Map<MeshId, MeshHandle>{&mesh_map_allocator_};
}

void GeometryManager::OnShutdown() {
  fiber::FiberLockGuard lock{mutex_};

  for (auto* mesh : meshes_) {
    if (mesh != nullptr) {
      DestroyMeshObject(mesh);
    }
  }

  meshes_.Release();
  mesh_handles_by_id_.Release();
  mesh_pool_.Destroy();

  index_allocator_.Destroy();
  vertex_allocator_.Destroy();
  mesh_map_allocator_.Destroy();
  mesh_array_allocator_.Destroy();
  mesh_allocator_.Destroy();
}

Mesh* GeometryManager::Get(MeshHandle handle) {
  auto* mesh{TryGet(handle)};
  COMET_ASSERT(mesh != nullptr, "GeometryManager::Get", "mesh does not exist",
               "handle", handle);
  return mesh;
}

Mesh* GeometryManager::TryGet(MeshHandle handle) {
  fiber::FiberLockGuard lock{mutex_};

  if (!mesh_pool_.IsAlive(handle)) {
    return nullptr;
  }

  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= meshes_.GetSize()) {
    return nullptr;
  }

  return meshes_[index];
}

Mesh* GeometryManager::CreateMeshObject(const resource::MeshResource& resource,
                                        MeshHandle handle) {
  COMET_PROFILE("GeometryManager::CreateMeshObject");
  auto* mesh{mesh_allocator_.AllocateOneAndPopulate<Mesh>()};

  mesh->handle = handle;
  mesh->id = GenerateMeshId(resource);
  mesh->type = resource.type;
  mesh->transform = resource.transform;
  mesh->local_center = resource.local_center;
  mesh->local_max_extents = resource.local_max_extents;
  mesh->ref_count = 1;

  mesh->indices = Array<Index>{&index_allocator_};
  mesh->indices.PushFromRange(resource.indices);

  mesh->vertices = Array<SkinnedVertex>{&vertex_allocator_};

  switch (resource.type) {
    case MeshType::Static: {
      const auto& static_mesh{
          static_cast<const resource::StaticMeshResource&>(resource)};
      mesh->vertices.PushFromRange(static_mesh.vertices);
      break;
    }

    case MeshType::Skinned: {
      const auto& skinned_mesh{
          static_cast<const resource::SkinnedMeshResource&>(resource)};
      mesh->vertices.PushFromRange(skinned_mesh.vertices);
      break;
    }

    case MeshType::Unknown:
    default:
      COMET_ASSERT(false, "GeometryManager::CreateMeshObject",
                   "unsupported mesh type", "mesh_type",
                   GetMeshTypeLabel(resource.type));
      break;
  }

  return mesh;
}

Mesh* GeometryManager::CreateProceduralMeshObject(
    MeshId mesh_id, MeshType type, const Array<SkinnedVertex>& vertices,
    const Array<Index>& indices, const math::Vec3& local_center,
    const math::Vec3& local_max_extents, MeshHandle handle) {
  COMET_PROFILE("GeometryManager::CreateProceduralMeshObject");
  auto* mesh{mesh_allocator_.AllocateOneAndPopulate<Mesh>()};

  mesh->handle = handle;
  mesh->id = mesh_id;
  mesh->type = type;
  mesh->transform = math::Mat4{1.0f};
  mesh->local_center = local_center;
  mesh->local_max_extents = local_max_extents;
  mesh->ref_count = 1;

  mesh->vertices = Array<SkinnedVertex>{&vertex_allocator_};
  mesh->vertices.PushFromRange(vertices);

  mesh->indices = Array<Index>{&index_allocator_};
  mesh->indices.PushFromRange(indices);

  return mesh;
}

void GeometryManager::DestroyMeshObject(Mesh* mesh) {
  COMET_PROFILE("GeometryManager::DestroyMeshObject");

  if (mesh == nullptr) {
    return;
  }

  mesh->vertices.Release();
  mesh->indices.Release();
  mesh_allocator_.Deallocate(mesh);
}

void GeometryManager::Destroy(MeshHandle handle, bool is_shutdown) {
  COMET_PROFILE("GeometryManager::Destroy");
  fiber::FiberLockGuard lock{mutex_};

  if (!mesh_pool_.IsAlive(handle)) {
    return;
  }

  const auto index{static_cast<usize>(handle.GetIndex())};
  COMET_ASSERT(index < meshes_.GetSize(), "GeometryManager::Destroy",
               "mesh handle index out of bounds", "handle", handle, "index",
               index, "mesh_count", meshes_.GetSize());

  auto* mesh{meshes_[index]};
  COMET_ASSERT(mesh != nullptr, "GeometryManager::Destroy", "mesh is null",
               "handle", handle, "index", index);

  if (!is_shutdown) {
    COMET_ASSERT(mesh->ref_count > 0, "GeometryManager::Destroy",
                 "mesh reference count is zero", "handle", handle, "mesh_id",
                 mesh->id);

    const auto ref_count{mesh->ref_count.fetch_sub(1) - 1};

    if (ref_count > 0) {
      return;
    }
  }

  meshes_[index] = nullptr;
  mesh_handles_by_id_.Remove(mesh->id);
  mesh_pool_.Destroy(handle);
  DestroyMeshObject(mesh);
}

}  // namespace geometry
}  // namespace comet