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
      mesh_pair_allocator_{sizeof(Pair<MeshId, Mesh*>), 1024,
                           memory::kEngineMemoryTagGeometry},
      vertex_allocator_{sizeof(Vertex), 1024, memory::kEngineMemoryTagGeometry},
      index_allocator_{sizeof(Index), 1024, memory::kEngineMemoryTagGeometry} {}

void GeometryManager::Initialize() {
  Manager::Initialize();
  mesh_allocator_.Initialize();
  mesh_pair_allocator_.Initialize();
  vertex_allocator_.Initialize();
  index_allocator_.Initialize();

  meshes_ = Map<MeshId, Mesh*>{&mesh_pair_allocator_};
}

void GeometryManager::Shutdown() {
  for (auto& it : meshes_) {
    Destroy(it.value, true);
  }

  meshes_.Destroy();

  index_allocator_.Destroy();
  vertex_allocator_.Destroy();
  mesh_pair_allocator_.Destroy();
  mesh_allocator_.Destroy();
  Manager::Shutdown();
}

Mesh* GeometryManager::Generate(const resource::StaticMeshResource* resource) {
  auto* mesh{GenerateInternal(resource)};
  mesh->vertices = Array<geometry::SkinnedVertex>{&vertex_allocator_};
  mesh->vertices.PushFromRange(resource->vertices);
  return mesh;
}

Mesh* GeometryManager::Generate(const resource::SkinnedMeshResource* resource) {
  auto* mesh{GenerateInternal(resource)};
  mesh->vertices = Array<geometry::SkinnedVertex>{&vertex_allocator_};
  mesh->vertices.Reserve(resource->vertices.GetSize());

  for (const auto& skinned_vertex : resource->vertices) {
    auto& vertex{mesh->vertices.EmplaceBack()};
    vertex.position = skinned_vertex.position;
    vertex.normal = skinned_vertex.normal;
    vertex.tangent = skinned_vertex.tangent;
    vertex.uv = skinned_vertex.uv;
    vertex.color = skinned_vertex.color;
    memory::CopyMemory(vertex.joint_indices, skinned_vertex.joint_indices,
                       sizeof(SkeletonJointIndex) * kMaxSkeletonJointCount);
    memory::CopyMemory(vertex.joint_weights, skinned_vertex.joint_weights,
                       sizeof(JointWeight) * kMaxSkeletonJointCount);
  }

  return mesh;
}

Mesh* GeometryManager::GenerateCube(f32 size) {
  COMET_ASSERT(size > .0f, "Debug cube size must be > 0!");

  const f32 h{size * .5f};

  Array<geometry::SkinnedVertex> vertices{&vertex_allocator_};
  Array<geometry::Index> indices{&index_allocator_};

  vertices.Reserve(24);
  indices.Reserve(36);

  auto push_vertex{[&](const math::Vec3& position, const math::Vec3& normal,
                       const math::Vec4& tangent, const math::Vec2& uv) {
    auto& vertex{vertices.EmplaceBack()};
    vertex.position = position;
    vertex.normal = normal;
    vertex.tangent = tangent;
    vertex.uv = uv;
    vertex.color = math::Vec4{1.0f};

    for (u32 i{0}; i < geometry::kMaxSkeletonJointCount; ++i) {
      vertex.joint_indices[i] = geometry::kInvalidSkeletonJointIndex;
      vertex.joint_weights[i] = .0f;
    }
  }};

  auto push_face{[&](const math::Vec3& p0, const math::Vec3& p1,
                     const math::Vec3& p2, const math::Vec3& p3,
                     const math::Vec3& normal, const math::Vec4& tangent) {
    auto base_index{static_cast<geometry::Index>(vertices.GetSize())};

    // Quad vertices in UV order.
    push_vertex(p0, normal, tangent, math::Vec2{.0f, .0f});
    push_vertex(p1, normal, tangent, math::Vec2{1.0f, .0f});
    push_vertex(p2, normal, tangent, math::Vec2{1.0f, 1.0f});
    push_vertex(p3, normal, tangent, math::Vec2{.0f, 1.0f});

    // Two triangles: (0, 1, 2) and (0, 2, 3).
    indices.PushBack(base_index + 0);
    indices.PushBack(base_index + 1);
    indices.PushBack(base_index + 2);

    indices.PushBack(base_index + 0);
    indices.PushBack(base_index + 2);
    indices.PushBack(base_index + 3);
  }};

  // +X face.
  push_face(math::Vec3{h, -h, -h}, math::Vec3{h, -h, h}, math::Vec3{h, h, h},
            math::Vec3{h, h, -h}, math::Vec3{1.0f, .0f, .0f},
            math::Vec4{.0f, .0f, 1.0f, 1.0f});

  // -X face.
  push_face(math::Vec3{-h, -h, h}, math::Vec3{-h, -h, -h},
            math::Vec3{-h, h, -h}, math::Vec3{-h, h, h},
            math::Vec3{-1.0f, .0f, .0f}, math::Vec4{.0f, .0f, -1.0f, 1.0f});

  // +Y face.
  push_face(math::Vec3{-h, h, -h}, math::Vec3{h, h, -h}, math::Vec3{h, h, h},
            math::Vec3{-h, h, h}, math::Vec3{.0f, 1.0f, .0f},
            math::Vec4{1.0f, .0f, .0f, 1.0f});

  // -Y face.
  push_face(math::Vec3{-h, -h, h}, math::Vec3{h, -h, h}, math::Vec3{h, -h, -h},
            math::Vec3{-h, -h, -h}, math::Vec3{.0f, -1.0f, .0f},
            math::Vec4{1.0f, .0f, .0f, 1.0f});

  // +Z face.
  push_face(math::Vec3{-h, -h, h}, math::Vec3{-h, h, h}, math::Vec3{h, h, h},
            math::Vec3{h, -h, h}, math::Vec3{.0f, .0f, 1.0f},
            math::Vec4{1.0f, .0f, .0f, 1.0f});

  // -Z face.
  push_face(math::Vec3{h, -h, -h}, math::Vec3{h, h, -h}, math::Vec3{-h, h, -h},
            math::Vec3{-h, -h, -h}, math::Vec3{.0f, .0f, -1.0f},
            math::Vec4{-1.0f, .0f, .0f, 1.0f});

  return GenerateProceduralMesh(internal::GetDebugUnitCubeMeshId(),
                                MeshType::Static, vertices, indices,
                                math::Vec3{.0f}, math::Vec3{h, h, h});
}

Mesh* GeometryManager::Get(MeshId mesh_id) {
  auto* mesh{TryGet(mesh_id)};
  COMET_ASSERT(mesh != nullptr, "Requested mesh does not exist: ", mesh_id,
               "!");
  return mesh;
}

Mesh* GeometryManager::Get(const resource::MeshResource* resource) {
  return Get(GenerateMeshId(resource));
}

Mesh* GeometryManager::TryGet(MeshId mesh_id) {
  Mesh** result;

  {
    fiber::FiberLockGuard lock{mutex_};
    result = meshes_.TryGet(mesh_id);
  }

  return result != nullptr ? *result : nullptr;
}

Mesh* GeometryManager::TryGet(const resource::MeshResource* resource) {
  return TryGet(GenerateMeshId(resource));
}

Mesh* GeometryManager::GetOrGenerate(const resource::MeshResource* resource) {
  auto* mesh{TryGet(resource)};

  if (mesh != nullptr) {
    ++mesh->ref_count;
    return mesh;
  }

  switch (resource->type) {
    case MeshType::Static:
      return Generate(
          static_cast<const resource::StaticMeshResource*>(resource));
    case MeshType::Skinned:
      return Generate(
          static_cast<const resource::SkinnedMeshResource*>(resource));
    default:
      COMET_ASSERT(false, "Unknown or unsupported mesh type: ",
                   GetMeshTypeLabel(resource->type), "!");
      return nullptr;
  }
}

void GeometryManager::Destroy(MeshId mesh_id) { Destroy(Get(mesh_id)); }

void GeometryManager::Destroy(Mesh* mesh) { Destroy(mesh, false); }

MeshId GeometryManager::GenerateMeshId(
    const resource::MeshResource* resource) const {
  return static_cast<u64>(resource->internal_id) |
         static_cast<u64>(resource->resource_id) << 32;
}

StaticModelComponent GeometryManager::GenerateStaticModelComponent(
    entity::EntityId entity_id, CTStringView model_path,
    resource::ResourceLifeSpan life_span) const {
  StaticModelComponent model_cmp{};
  model_cmp.entity_id = entity_id;
  model_cmp.resource = resource::ResourceManager::Get().GetStaticModels()->Load(
      model_path, life_span);
  return model_cmp;
}

SkeletalModelComponent GeometryManager::GenerateSkeletalModelComponent(
    entity::EntityId entity_id, CTStringView model_path,
    resource::ResourceLifeSpan life_span) const {
  SkeletalModelComponent model_cmp{};
  model_cmp.entity_id = entity_id;
  model_cmp.resource =
      resource::ResourceManager::Get().GetSkeletalModels()->Load(model_path,
                                                                 life_span);
  return model_cmp;
}

MeshComponent GeometryManager::GenerateStaticMeshComponent(
    const resource::StaticMeshResource* resource, entity::EntityId entity_id,
    entity::EntityId model_entity_id, resource::ResourceLifeSpan life_span) {
  MeshComponent mesh_cmp{};
  mesh_cmp.entity_id = entity_id;
  mesh_cmp.model_entity_id = model_entity_id;
  mesh_cmp.mesh = GetOrGenerate(resource);
  mesh_cmp.material_resource =
      resource::ResourceManager::Get().GetMaterials()->Load(
          resource->material_id, life_span);

  return mesh_cmp;
}

MeshComponent GeometryManager::GenerateSkinnedMeshComponent(
    const resource::SkinnedMeshResource* resource, entity::EntityId entity_id,
    entity::EntityId model_entity_id, resource::ResourceLifeSpan life_span) {
  MeshComponent mesh_cmp{};
  mesh_cmp.entity_id = entity_id;
  mesh_cmp.model_entity_id = model_entity_id;
  mesh_cmp.mesh = GetOrGenerate(resource);
  mesh_cmp.material_resource =
      resource::ResourceManager::Get().GetMaterials()->Load(
          resource->material_id, life_span);
  return mesh_cmp;
}

SkeletonComponent GeometryManager::GenerateSkeletonComponent(
    CTStringView model_path, resource::ResourceLifeSpan life_span) const {
  SkeletonComponent skeleton_cmp{};
  skeleton_cmp.resource = resource::ResourceManager::Get().GetSkeletons()->Load(
      model_path, life_span);
  return skeleton_cmp;
}

void GeometryManager::DestroyStaticModelComponent(
    StaticModelComponent* model_cmp) const {
  model_cmp->entity_id = entity::kInvalidEntityId;

  if (model_cmp->resource != nullptr) {
    resource::ResourceManager::Get().GetStaticModels()->Unload(
        model_cmp->resource->id);
    model_cmp->resource = nullptr;
  }
}

void GeometryManager::DestroySkeletalModelComponent(
    SkeletalModelComponent* model_cmp) const {
  model_cmp->entity_id = entity::kInvalidEntityId;

  if (model_cmp->resource != nullptr) {
    resource::ResourceManager::Get().GetSkeletalModels()->Unload(
        model_cmp->resource->id);
    model_cmp->resource = nullptr;
  }
}

void GeometryManager::DestroyStaticMeshComponent(MeshComponent* mesh_cmp) {
  mesh_cmp->entity_id = entity::kInvalidEntityId;
  mesh_cmp->model_entity_id = entity::kInvalidEntityId;

  if (mesh_cmp->mesh != nullptr) {
    Destroy(mesh_cmp->mesh);
    mesh_cmp->mesh = nullptr;
  }

  if (mesh_cmp->material_resource != nullptr) {
    resource::ResourceManager::Get().GetMaterials()->Unload(
        mesh_cmp->material_resource->id);
  }

  mesh_cmp->material_resource = nullptr;
}

void GeometryManager::DestroySkinnedMeshComponent(MeshComponent* mesh_cmp) {
  mesh_cmp->entity_id = entity::kInvalidEntityId;
  mesh_cmp->model_entity_id = entity::kInvalidEntityId;

  if (mesh_cmp->mesh != nullptr) {
    Destroy(mesh_cmp->mesh);
    mesh_cmp->mesh = nullptr;
  }

  if (mesh_cmp->material_resource != nullptr) {
    resource::ResourceManager::Get().GetMaterials()->Unload(
        mesh_cmp->material_resource->id);
  }

  mesh_cmp->material_resource = nullptr;
}

void GeometryManager::DestroySkeletonComponent(
    SkeletonComponent* skeleton_cmp) {
  if (skeleton_cmp->resource != nullptr) {
    resource::ResourceManager::Get().GetSkeletons()->Unload(
        skeleton_cmp->resource->id);
  }

  skeleton_cmp->resource = nullptr;
}

Mesh* GeometryManager::GenerateInternal(
    const resource::MeshResource* resource) {
  auto* mesh{mesh_allocator_.AllocateOneAndPopulate<Mesh>()};

  mesh->id = GenerateMeshId(resource);
  mesh->type = resource->type;
  mesh->transform = resource->transform;
  mesh->local_center = resource->local_center;
  mesh->local_max_extents = resource->local_max_extents;

  mesh->indices = Array<geometry::Index>{&index_allocator_};
  mesh->indices.PushFromRange(resource->indices);

#ifdef COMET_DEBUG
  auto mesh_id{mesh->id};
#endif  // COMET_DEBUG

  Mesh* to_return;

  {
    fiber::FiberLockGuard lock{mutex_};
    auto& insert_pair{meshes_.Emplace(mesh->id, std::move(mesh))};
    to_return = insert_pair.value;
  }

  if (to_return == nullptr) {
    COMET_ASSERT(false, "Could not insert mesh #", mesh_id, "!");
  }

  to_return->ref_count = 1;
  return to_return;
}

Mesh* GeometryManager::GenerateProceduralMesh(
    MeshId mesh_id, MeshType type,
    const Array<geometry::SkinnedVertex>& vertices,
    const Array<geometry::Index>& indices, const math::Vec3& local_center,
    const math::Vec3& local_max_extents) {
  if (auto* existing{TryGet(mesh_id)}; existing != nullptr) {
    ++existing->ref_count;
    return existing;
  }

  auto* mesh{mesh_allocator_.AllocateOneAndPopulate<Mesh>()};

  mesh->id = mesh_id;
  mesh->type = type;
  mesh->transform = math::Mat4{1.0f};
  mesh->local_center = local_center;
  mesh->local_max_extents = local_max_extents;

  mesh->vertices = Array<geometry::SkinnedVertex>{&vertex_allocator_};
  mesh->vertices.PushFromRange(vertices);

  mesh->indices = Array<geometry::Index>{&index_allocator_};
  mesh->indices.PushFromRange(indices);

#ifdef COMET_DEBUG
  auto generated_mesh_id{mesh->id};
#endif  // COMET_DEBUG

  Mesh* to_return{nullptr};

  {
    fiber::FiberLockGuard lock{mutex_};
    auto& insert_pair{meshes_.Emplace(mesh->id, std::move(mesh))};
    to_return = insert_pair.value;
  }

  if (to_return == nullptr) {
    COMET_ASSERT(false, "Could not insert procedural mesh #", generated_mesh_id,
                 "!");
  }

  to_return->ref_count = 1;
  return to_return;
}

void GeometryManager::Destroy(Mesh* mesh, bool is_destroying_handler) {
  COMET_ASSERT(mesh != nullptr, "Mesh provided is null!");
  COMET_ASSERT(mesh->ref_count > 0, "Mesh has a reference count of 0!");

  if (is_destroying_handler) {
    return;
  }

  auto ref_count{mesh->ref_count.fetch_sub(1) - 1};

  if (ref_count > 0) {
    return;
  }

  {
    fiber::FiberLockGuard lock{mutex_};
    meshes_.Remove(mesh->id);
  }

  mesh_allocator_.Deallocate(mesh);
}
}  // namespace geometry
}  // namespace comet
