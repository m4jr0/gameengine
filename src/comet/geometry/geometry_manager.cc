// Copyright 2023 m4jr0. All Rights Reserved.
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

void GeometryManager::Shutdown() {
  for (auto& it : meshes_) {
    auto& mesh{it.second};
    Destroy(it.second, true);
  }

  meshes_.clear();
  Manager::Shutdown();
}

Mesh* GeometryManager::Generate(const resource::StaticMeshResource* resource) {
  auto* mesh{GenerateInternal(resource)};
  mesh->vertices = resource->vertices;
  return mesh;
}

Mesh* GeometryManager::Generate(const resource::SkinnedMeshResource* resource) {
  auto* mesh{GenerateInternal(resource)};
  mesh->vertices.reserve(resource->vertices.size());

  for (const auto& skinned_vertex : resource->vertices) {
    Vertex vertex{};
    vertex.position = skinned_vertex.position;
    vertex.normal = skinned_vertex.normal;
    vertex.tangent = skinned_vertex.tangent;
    vertex.bitangent = skinned_vertex.bitangent;
    vertex.uv = skinned_vertex.uv;
    vertex.color = skinned_vertex.color;
    mesh->vertices.push_back(std::move(vertex));
  }

  return mesh;
}

Mesh* GeometryManager::Get(MeshId mesh_id, geometry::MeshType type) {
  auto* mesh{TryGet(mesh_id, type)};
  COMET_ASSERT(mesh != nullptr, "Requested mesh does not exist: ", mesh_id,
               "!");
  return mesh;
}

Mesh* GeometryManager::Get(const resource::MeshResource* resource) {
  return Get(GenerateMeshId(resource), resource->type);
}

Mesh* GeometryManager::TryGet(MeshId mesh_id, geometry::MeshType type) {
  auto it{meshes_.find(mesh_id)};

  if (it != meshes_.end()) {
    return &it->second;
  }

  return nullptr;
}

Mesh* GeometryManager::TryGet(const resource::MeshResource* resource) {
  return TryGet(GenerateMeshId(resource), resource->type);
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
  }

  COMET_ASSERT(false, "Unknown or unsupported mesh type: ",
               GetMeshTypeLabel(resource->type), "!");
  return nullptr;
}

void GeometryManager::Destroy(MeshId mesh_id, geometry::MeshType type) {
  Destroy(*Get(mesh_id, type));
}

void GeometryManager::Destroy(Mesh& mesh) { Destroy(mesh, false); }

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
  Mesh mesh{};

  mesh.id = GenerateMeshId(resource);
  mesh.type = resource->type;
  mesh.transform = resource->transform;
  mesh.local_center = resource->local_center;
  mesh.local_max_extents = resource->local_max_extents;
  mesh.indices = resource->indices;

#ifdef COMET_DEBUG
  const auto mesh_id{mesh.id};
#endif  // COMET_DEBUG

  const auto insert_pair{meshes_.emplace(mesh.id, std::move(mesh))};
  Mesh* to_return{insert_pair.second ? &insert_pair.first->second : nullptr};

  if (to_return == nullptr) {
    COMET_ASSERT(
        false, "Could not insert mesh: ", COMET_STRING_ID_LABEL(mesh_id), "!");
  }

  return to_return;
}

void GeometryManager::Destroy(Mesh& mesh, bool is_destroying_handler) {
  mesh.type = MeshType::Unknown;
  mesh.indices.clear();
  mesh.vertices.clear();

  if (!is_destroying_handler) {
    meshes_.erase(mesh.id);
  }

  mesh.id = kInvalidMeshId;
}
}  // namespace geometry
}  // namespace comet
