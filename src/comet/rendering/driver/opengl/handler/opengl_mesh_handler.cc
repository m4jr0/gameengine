// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "opengl_mesh_handler.h"

#include "comet/core/logger.h"
#include "comet/profiler/profiler.h"

namespace comet {
namespace rendering {
namespace gl {
MeshHandler::MeshHandler(const MeshHandlerDescr& descr) : Handler{descr} {}

void MeshHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();
  mesh_to_proxy_map_ = {&allocator_, kDefaultProxyCount_};
  proxies_ = {&allocator_, kDefaultProxyCount_};

  vertex_buffer_ = VertexGpuBuffer{&allocator_, kVertexCountPerBlock_,
                                   kDefaultVertexCount_, 0, "vertex_buffer_"};
  index_buffer_ = IndexGpuBuffer{&allocator_, kIndexCountPerBlock_,
                                 kDefaultIndexCount_, 0, "index_buffer_"};

  vertex_buffer_.Initialize();
  index_buffer_.Initialize();
}

void MeshHandler::Shutdown() {
  proxies_.Destroy();
  mesh_to_proxy_map_.Destroy();
  vertex_buffer_.Destroy();
  index_buffer_.Destroy();
  allocator_.Destroy();
  Handler::Shutdown();
}

void MeshHandler::Update(const frame::FramePacket* packet) {
  COMET_PROFILE("MeshHandler::Update");

  if (packet->added_geometries->IsEmpty() && packet->dirty_meshes->IsEmpty() &&
      packet->removed_geometries->IsEmpty()) {
    return;
  }

  // Removing meshes is CPU-side only.
  DestroyMeshProxies(packet->removed_geometries);
  auto update_context{PrepareUpdate(packet)};
  UpdateMeshProxies(packet->dirty_meshes, update_context);
  AddMeshProxies(packet->added_geometries, update_context);
  FinishUpdate(update_context);
}

void MeshHandler::Bind() {
  vertex_buffer_.Bind();
  index_buffer_.Bind();
}

MeshProxyHandle MeshHandler::GetHandle(geometry::MeshId mesh_id) const {
  auto* index{mesh_to_proxy_map_.TryGet(mesh_id)};
  return index != nullptr ? static_cast<MeshProxyHandle>(*index)
                          : kInvalidMeshProxyHandle;
}

const MeshProxy* MeshHandler::Get(MeshProxyHandle handle) const {
  return &proxies_[static_cast<usize>(handle)];
}

StorageHandle MeshHandler::GetVertexBufferHandle() const {
  return vertex_buffer_.GetHandle();
}

StorageHandle MeshHandler::GetIndexBufferHandle() const {
  return index_buffer_.GetHandle();
}

internal::UpdateContext MeshHandler::PrepareUpdate(
    const frame::FramePacket* packet) {
  COMET_PROFILE("MeshHandler::PrepareUpdate");
  internal::UpdateContext update_context{};

  for (const auto& geometry : *packet->added_geometries) {
    update_context.new_vertex_size += static_cast<GLsizei>(
        geometry.vertices->GetSize() * sizeof(geometry::SkinnedVertex));
    update_context.new_index_size += static_cast<GLsizei>(
        geometry.indices->GetSize() * sizeof(geometry::Index));
  }

  for (const auto& mesh : *packet->dirty_meshes) {
    update_context.dirty_vertex_size += static_cast<GLsizei>(
        mesh.vertices->GetSize() * sizeof(geometry::SkinnedVertex));
    update_context.dirty_index_size +=
        static_cast<GLsizei>(mesh.indices->GetSize() * sizeof(geometry::Index));
  }

  update_context.total_vertex_size =
      update_context.new_vertex_size + update_context.dirty_vertex_size;
  update_context.total_index_size =
      update_context.new_index_size + update_context.dirty_index_size;

  vertex_buffer_.Resize(update_context.total_vertex_size /
                        sizeof(geometry::SkinnedVertex));
  index_buffer_.Resize(update_context.total_index_size /
                       sizeof(geometry::Index));

  auto total_size{update_context.total_vertex_size +
                  update_context.total_index_size};

  auto upload_count{packet->added_geometries->GetSize() +
                    packet->dirty_meshes->GetSize()};

  update_context.vertex_copy_regions.Reserve(upload_count);
  update_context.index_copy_regions.Reserve(upload_count);

  // Indices are copied at the end of the staging buffer.
  update_context.current_staging_index_offset =
      update_context.total_vertex_size;

  update_context.staging_buffer =
      total_size > 0 ? COMET_FRAME_ALLOC(total_size) : nullptr;
  return update_context;
}

void MeshHandler::FinishUpdate(internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::FinishUpdate");
  UploadMeshProxies(update_context);
  update_context.staging_buffer = nullptr;
}

void MeshHandler::AddMeshProxies(const frame::AddedGeometries* geometries,
                                 internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::AddMeshProxies");
  if (geometries->IsEmpty()) {
    return;
  }

  auto* memory{static_cast<u8*>(update_context.staging_buffer)};

  for (const auto& geometry : *geometries) {
    auto vertex_size{static_cast<GLsizei>(geometry.vertices->GetSize() *
                                          sizeof(geometry::SkinnedVertex))};
    auto index_size{static_cast<GLsizei>(geometry.indices->GetSize() *
                                         sizeof(geometry::Index))};

    memory::CopyMemory(memory + update_context.current_staging_vertex_offset,
                       geometry.vertices->GetData(), vertex_size);
    memory::CopyMemory(memory + update_context.current_staging_index_offset,
                       geometry.indices->GetData(), index_size);

    auto vertex_offset{vertex_buffer_.Claim(geometry.vertices->GetSize())};
    auto index_offset{index_buffer_.Claim(geometry.indices->GetSize())};

    update_context.vertex_copy_regions.EmplaceBack(
        update_context.current_staging_vertex_offset,
        static_cast<GLsizeiptr>(vertex_offset *
                                sizeof(geometry::SkinnedVertex)),
        vertex_size);
    update_context.index_copy_regions.EmplaceBack(
        update_context.current_staging_index_offset,
        static_cast<GLsizeiptr>(index_offset * sizeof(geometry::Index)),
        index_size);

    update_context.current_staging_vertex_offset += vertex_size;
    update_context.current_staging_index_offset += index_size;

    mesh_to_proxy_map_[geometry.mesh_id] = proxies_.GetSize();
    auto& proxy{proxies_.EmplaceBack()};
    proxy.vertex_count = static_cast<u32>(geometry.vertices->GetSize());
    proxy.index_count = static_cast<u32>(geometry.indices->GetSize());
    proxy.vertex_offset = static_cast<GLint>(vertex_offset);
    proxy.index_offset = static_cast<GLint>(index_offset);
  }
}

void MeshHandler::UpdateMeshProxies(const frame::DirtyMeshes* meshes,
                                    internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::UpdateMeshProxies");
  if (meshes->IsEmpty()) {
    return;
  }

  auto* memory{static_cast<u8*>(update_context.staging_buffer)};

  for (const auto& mesh : *meshes) {
    auto* proxy_id{mesh_to_proxy_map_.TryGet(mesh.mesh_id)};

    if (proxy_id == nullptr) {
      COMET_LOG_RENDERING_WARNING("Tried to update non-existing mesh #",
                                  mesh.mesh_id, "!");
      continue;
    }

    COMET_ASSERT(*proxy_id < proxies_.GetSize(), "Proxy index out of bounds!");
    auto& proxy{proxies_[*proxy_id]};

    auto new_vertex_size{static_cast<GLsizei>(mesh.vertices->GetSize() *
                                              sizeof(geometry::SkinnedVertex))};
    auto new_index_size{static_cast<GLsizei>(mesh.indices->GetSize() *
                                             sizeof(geometry::Index))};

    auto vertex_offset{vertex_buffer_.CheckOrMove(
        proxy.vertex_offset, proxy.vertex_count, new_vertex_size)};

    auto index_offset{index_buffer_.CheckOrMove(
        proxy.index_offset, proxy.index_count, new_index_size)};

    proxy.vertex_count = static_cast<u32>(mesh.vertices->GetSize());
    proxy.index_count = static_cast<u32>(mesh.indices->GetSize());
    proxy.vertex_offset = static_cast<GLint>(vertex_offset);
    proxy.index_offset = static_cast<GLint>(index_offset);

    memory::CopyMemory(memory + update_context.current_staging_vertex_offset,
                       mesh.vertices->GetData(), new_vertex_size);
    memory::CopyMemory(memory + update_context.current_staging_index_offset,
                       mesh.indices->GetData(), new_index_size);

    update_context.vertex_copy_regions.EmplaceBack(
        update_context.current_staging_vertex_offset,
        static_cast<GLsizeiptr>(vertex_offset *
                                sizeof(geometry::SkinnedVertex)),
        new_vertex_size);
    update_context.index_copy_regions.EmplaceBack(
        update_context.current_staging_index_offset,
        static_cast<GLsizeiptr>(index_offset * sizeof(geometry::Index)),
        new_index_size);

    update_context.current_staging_vertex_offset += new_vertex_size;
    update_context.current_staging_index_offset += new_index_size;
  }
}

void MeshHandler::DestroyMeshProxies(
    const frame::RemovedGeometries* geometries) {
  COMET_PROFILE("MeshHandler::DestroyMeshProxies");

  for (const auto& geometry : *geometries) {
    auto* proxy_id{mesh_to_proxy_map_.TryGet(geometry.mesh_id)};

    if (proxy_id == nullptr) {
      COMET_LOG_RENDERING_WARNING("Tried to remove non-existing mesh #",
                                  geometry.mesh_id, "!");
      continue;
    }

    auto& proxy{proxies_[*proxy_id]};
    vertex_buffer_.Release(proxy.vertex_offset, proxy.vertex_count);
    index_buffer_.Release(proxy.index_offset, proxy.index_count);
    mesh_to_proxy_map_.Remove(geometry.mesh_id);
  }
}

void MeshHandler::UploadMeshProxies(
    const internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::UploadMeshProxies");
  vertex_buffer_.Upload(update_context.staging_buffer,
                        update_context.vertex_copy_regions);
  index_buffer_.Upload(update_context.staging_buffer,
                       update_context.index_copy_regions);
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
