// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "geometry_manager.h"

#include "comet/resource/material_resource.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace geometry {
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

  meshes_.Clear();

  index_allocator_.Destroy();
  vertex_allocator_.Destroy();
  mesh_pair_allocator_.Destroy();
  mesh_allocator_.Destroy();
  Manager::Shutdown();
}

Mesh* GeometryManager::Generate(const resource::StaticMeshResource* resource) {
  auto* mesh{GenerateInternal(resource)};
  mesh->vertices = Array<geometry::Vertex>{&vertex_allocator_};
  mesh->vertices.PushFromRange(resource->vertices);
  return mesh;
}

Mesh* GeometryManager::Generate(const resource::SkinnedMeshResource* resource) {
  auto* mesh{GenerateInternal(resource)};
  mesh->vertices = Array<geometry::Vertex>{&vertex_allocator_};
  mesh->vertices.Reserve(resource->vertices.GetSize());

  for (const auto& skinned_vertex : resource->vertices) {
    Vertex vertex{};
    vertex.position = skinned_vertex.position;
    vertex.normal = skinned_vertex.normal;
    vertex.tangent = skinned_vertex.tangent;
    vertex.bitangent = skinned_vertex.bitangent;
    vertex.uv = skinned_vertex.uv;
    vertex.color = skinned_vertex.color;
    mesh->vertices.PushBack(std::move(vertex));
  }

  return mesh;
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

MeshComponent GeometryManager::GenerateComponent(
    const resource::StaticMeshResource* resource) {
  MeshComponent mesh_cmp{};

  mesh_cmp.mesh = GetOrGenerate(resource);
  mesh_cmp.material =
      resource::ResourceManager::Get().Load<resource::MaterialResource>(
          resource->material_id);
  return mesh_cmp;
}

SkeletalComponents GeometryManager::GenerateComponents(
    const resource::SkinnedMeshResource* resource) {
  SkeletalComponents skeletal_cmps{};

  skeletal_cmps.mesh_cmp.mesh = GetOrGenerate(resource);
  skeletal_cmps.mesh_cmp.material =
      resource::ResourceManager::Get().Load<resource::MaterialResource>(
          resource->material_id);

  skeletal_cmps.skeleton_cmp.resource = resource;
  return skeletal_cmps;
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
  const auto mesh_id{mesh->id};
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

  return to_return;
}

void GeometryManager::Destroy(Mesh* mesh, bool is_destroying_handler) {
  COMET_ASSERT(mesh != nullptr, "Mesh provided is null!");
  if (!is_destroying_handler) {
    {
      fiber::FiberLockGuard lock{mutex_};
      meshes_.Remove(mesh->id);
    }

    mesh_allocator_.Deallocate(mesh);
  }
}
}  // namespace geometry
}  // namespace comet
