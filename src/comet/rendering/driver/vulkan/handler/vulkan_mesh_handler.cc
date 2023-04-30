// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_mesh_handler.h"

#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
MeshHandler::MeshHandler(const MeshHandlerDescr& descr) : Handler{descr} {}

void MeshHandler::Shutdown() {
  for (auto& it : meshes_) {
    Destroy(it.second, true);
  }

  meshes_.clear();
  Handler::Shutdown();
}

Mesh* MeshHandler::Generate(const resource::MeshResource* resource) {
  Mesh mesh{};
  mesh.id = GenerateMeshId(resource);

  const auto vertex_count{resource->vertices.size()};
  mesh.vertices.resize(resource->vertices.size());

  for (uindex i{0}; i < vertex_count; ++i) {
    auto& vertex{mesh.vertices[i]};
    auto& vertex_res{resource->vertices[i]};

    vertex.position = vertex_res.position;
    vertex.normal = vertex_res.normal;

    vertex.uv.x = vertex_res.uv.x;
    vertex.uv.y = vertex_res.uv.y;

    vertex.color = {kColorWhite, 1.0f};
  }

  const auto index_count{resource->indices.size()};
  mesh.indices.resize(resource->indices.size());
  mesh.vertex_buffer.allocator_handle = context_->GetAllocatorHandle();

  for (uindex i{0}; i < index_count; ++i) {
    mesh.indices[i] = resource->indices[i];
  }

#ifdef COMET_DEBUG
  const auto mesh_id{mesh.id};
#endif  // COMET_DEBUG

  const auto insert_pair{meshes_.emplace(mesh.id, std::move(mesh))};
  COMET_ASSERT(insert_pair.second,
               "Could not insert mesh: ", COMET_STRING_ID_LABEL(mesh_id), "!");
  auto& to_return{insert_pair.first->second};
  Upload(to_return);
  return &to_return;
}

Mesh* MeshHandler::Get(MeshId mesh_id) {
  auto* mesh{TryGet(mesh_id)};
  COMET_ASSERT(mesh != nullptr, "Requested mesh does not exist: ", mesh_id,
               "!");
  return mesh;
}

Mesh* MeshHandler::Get(const resource::MeshResource* resource) {
  return Get(GenerateMeshId(resource));
}

Mesh* MeshHandler::TryGet(MeshId mesh_id) {
  auto it{meshes_.find(mesh_id)};

  if (it == meshes_.end()) {
    return nullptr;
  }

  return &it->second;
}

Mesh* MeshHandler::TryGet(const resource::MeshResource* resource) {
  return TryGet(GenerateMeshId(resource));
}

Mesh* MeshHandler::GetOrGenerate(const resource::MeshResource* resource) {
  auto* mesh{TryGet(resource)};

  if (mesh != nullptr) {
    return mesh;
  }

  return Generate(resource);
}

void MeshHandler::Destroy(MeshId mesh_id) { Destroy(*Get(mesh_id)); }

void MeshHandler::Destroy(Mesh& mesh) { Destroy(mesh, false); }

void MeshHandler::Upload(Mesh& mesh) const {
  const auto vertex_buffer_size{sizeof(Vertex) * mesh.vertices.size()};
  const auto index_buffer_size{sizeof(Index) * mesh.indices.size()};
  const auto buffer_size{vertex_buffer_size + index_buffer_size};
  auto allocator_handle{context_->GetAllocatorHandle()};

  Buffer staging_buffer{GenerateBuffer(
      allocator_handle, buffer_size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_AUTO,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT)};

  MapBuffer(staging_buffer);
  CopyToBuffer(staging_buffer, mesh.vertices.data(), vertex_buffer_size);
  CopyToBuffer(staging_buffer, mesh.indices.data(), index_buffer_size,
               vertex_buffer_size);
  UnmapBuffer(staging_buffer);

  mesh.vertex_buffer = GenerateBuffer(
      allocator_handle, buffer_size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_AUTO, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  CopyBuffer(context_->GetDevice(),
             context_->GetUploadContext().command_pool_handle, staging_buffer,
             mesh.vertex_buffer, buffer_size,
             context_->GetUploadContext().upload_fence_handle);
  DestroyBuffer(staging_buffer);
}

MeshId MeshHandler::GenerateMeshId(
    const resource::MeshResource* resource) const {
  return static_cast<u64>(resource->internal_id) |
         static_cast<u64>(resource->resource_id) << 32;
}

void MeshHandler::Destroy(Mesh& mesh, bool is_destroying_handler) {
  if (mesh.vertex_buffer.allocation_handle != VK_NULL_HANDLE) {
    DestroyBuffer(mesh.vertex_buffer);
  }

  mesh.vertices.clear();
  mesh.indices.clear();
  mesh.vertex_buffer = {};

  if (!is_destroying_handler) {
    meshes_.erase(mesh.id);
  }

  mesh.id = kInvalidMeshId;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
