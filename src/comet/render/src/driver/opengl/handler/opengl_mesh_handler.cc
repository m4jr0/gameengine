// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/opengl/handler/opengl_mesh_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/logger/logging.h"
#include "comet/profiler/profiler.h"

namespace comet {
namespace rendering {
namespace gl {
MeshHandler::MeshHandler(const MeshHandlerDescr& descr)
    : Handler{descr}, proxies_{&registry_allocator_, kDefaultProxyCount_} {}

void MeshHandler::Update(const frame::FramePacket* packet) {
  COMET_PROFILE("MeshHandler::Update");
  COMET_ASSERT(packet != nullptr, "MeshHandler::Update",
               "frame packet is null");

  if (packet->added_geometries->IsEmpty() && packet->dirty_meshes->IsEmpty() &&
      packet->removed_geometries->IsEmpty()) {
    return;
  }

  DestroyMeshProxies(packet->removed_geometries);
  auto update_context{PrepareUpdate(packet)};
  UpdateMeshProxies(packet->dirty_meshes, update_context);
  AddMeshProxies(packet->added_geometries, update_context);
  FinishUpdate(update_context);
}

const MeshProxy* MeshHandler::Get(geometry::MeshHandle handle) const {
  const auto* proxy{TryGet(handle)};
  COMET_ASSERT(proxy != nullptr, "MeshHandler::Get", "mesh proxy not found",
               "mesh_handle", handle);
  return proxy;
}

const MeshProxy* MeshHandler::TryGet(geometry::MeshHandle handle) const {
  if (!handle) {
    return nullptr;
  }

  const auto proxy_handle{proxies_.TryGetHandle(handle)};

  if (!proxy_handle) {
    return nullptr;
  }

  return proxies_.TryGet(proxy_handle);
}

GlNativeStorageHandle MeshHandler::GetVertexBufferHandle() const {
  return vertex_buffer_.GetHandle();
}

GlNativeStorageHandle MeshHandler::GetIndexBufferHandle() const {
  return index_buffer_.GetHandle();
}

ShaderVertexSource MeshHandler::GetVertexSource() const {
  ShaderVertexSource source{};
  source.vertex_buffer_native_handle = GetVertexBufferHandle();
  source.index_buffer_native_handle = GetIndexBufferHandle();
  source.has_index_buffer = true;
  source.vertex_source_id =
      (static_cast<u64>(source.vertex_buffer_native_handle) << 32) |
      static_cast<u64>(source.index_buffer_native_handle);
  return source;
}

void MeshHandler::OnInitialize() {
  allocator_.Initialize();
  proxies_.Initialize();

  vertex_buffer_ = VertexGpuBuffer{&allocator_, kVertexCountPerBlock_,
                                   kDefaultVertexCount_, 0, "vertex_buffer_"};
  index_buffer_ = IndexGpuBuffer{&allocator_, kIndexCountPerBlock_,
                                 kDefaultIndexCount_, 0, "index_buffer_"};

  vertex_buffer_.Initialize();
  index_buffer_.Initialize();
}

void MeshHandler::OnShutdown() {
  memory::PlatformAllocator tmp_allocator{memory::kEngineMemoryTagRendering};

  auto handles_to_destroy{Array<MeshProxyRegistry::ItemHandle>::WithCapacity(
      &tmp_allocator, proxies_.GetLiveCount())};

  proxies_.ForEachLive(
      [&handles_to_destroy](MeshProxyRegistry::ItemHandle handle,
                            const MeshProxy*) {
        handles_to_destroy.PushLast(handle);
      });

  for (const auto handle : handles_to_destroy) {
    auto* proxy{proxies_.Drain(handle)};

    if (proxy == nullptr) {
      continue;
    }

    vertex_buffer_.Release(proxy->vertex_offset, proxy->vertex_count);
    index_buffer_.Release(proxy->index_offset, proxy->index_count);

    allocator_.Deallocate(proxy);
  }

  proxies_.Destroy();

  vertex_buffer_.Destroy();
  index_buffer_.Destroy();

  allocator_.Destroy();
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

  const auto total_size{update_context.total_vertex_size +
                        update_context.total_index_size};

  const auto upload_count{packet->added_geometries->GetSize() +
                          packet->dirty_meshes->GetSize()};

  update_context.vertex_copy_regions.Reserve(upload_count);
  update_context.index_copy_regions.Reserve(upload_count);

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
    if (!geometry.mesh_handle) {
      COMET_LOG_WARNING(LoggerType::Rendering, "MeshHandler::AddMeshProxies",
                        "invalid mesh handle");
      continue;
    }

    if (const auto proxy_handle{proxies_.TryAcquire(geometry.mesh_handle)};
        proxy_handle) {
      continue;
    }

    const auto vertex_size{static_cast<GLsizei>(
        geometry.vertices->GetSize() * sizeof(geometry::SkinnedVertex))};
    const auto index_size{static_cast<GLsizei>(geometry.indices->GetSize() *
                                               sizeof(geometry::Index))};

    memory::CopyMemory(memory + update_context.current_staging_vertex_offset,
                       geometry.vertices->GetData(), vertex_size);
    memory::CopyMemory(memory + update_context.current_staging_index_offset,
                       geometry.indices->GetData(), index_size);

    const auto vertex_offset{
        vertex_buffer_.Claim(geometry.vertices->GetSize())};
    const auto index_offset{index_buffer_.Claim(geometry.indices->GetSize())};

    update_context.vertex_copy_regions.EmplaceLast(
        update_context.current_staging_vertex_offset,
        static_cast<GLsizeiptr>(vertex_offset *
                                sizeof(geometry::SkinnedVertex)),
        vertex_size);
    update_context.index_copy_regions.EmplaceLast(
        update_context.current_staging_index_offset,
        static_cast<GLsizeiptr>(index_offset * sizeof(geometry::Index)),
        index_size);

    update_context.current_staging_vertex_offset += vertex_size;
    update_context.current_staging_index_offset += index_size;

    auto* proxy{allocator_.AllocateOneAndPopulate<MeshProxy>()};

    proxy->is_alive = true;
    proxy->mesh_handle = geometry.mesh_handle;
    proxy->vertex_count = static_cast<GLsizei>(geometry.vertices->GetSize());
    proxy->index_count = static_cast<GLsizei>(geometry.indices->GetSize());
    proxy->vertex_offset = static_cast<GLint>(vertex_offset);
    proxy->index_offset = static_cast<GLint>(index_offset);

    const auto proxy_handle{proxies_.Create(geometry.mesh_handle, proxy)};
    COMET_ASSERT(proxy_handle, "MeshHandler::AddMeshProxies",
                 "mesh proxy registry creation failed", "mesh_handle",
                 geometry.mesh_handle);
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
    if (!mesh.mesh_handle) {
      COMET_LOG_WARNING(LoggerType::Rendering, "MeshHandler::UpdateMeshProxies",
                        "invalid mesh handle");
      continue;
    }

    const auto proxy_handle{proxies_.TryGetHandle(mesh.mesh_handle)};

    if (!proxy_handle) {
      COMET_LOG_WARNING(LoggerType::Rendering, "MeshHandler::UpdateMeshProxies",
                        "mesh proxy not found", "mesh_handle",
                        mesh.mesh_handle);
      continue;
    }

    auto* proxy{proxies_.Get(proxy_handle)};

    const auto new_vertex_size{static_cast<GLsizei>(
        mesh.vertices->GetSize() * sizeof(geometry::SkinnedVertex))};
    const auto new_index_size{static_cast<GLsizei>(mesh.indices->GetSize() *
                                                   sizeof(geometry::Index))};

    const auto vertex_offset{vertex_buffer_.CheckOrMove(
        proxy->vertex_offset, proxy->vertex_count, mesh.vertices->GetSize())};
    const auto index_offset{index_buffer_.CheckOrMove(
        proxy->index_offset, proxy->index_count, mesh.indices->GetSize())};

    proxy->vertex_count = static_cast<GLsizei>(mesh.vertices->GetSize());
    proxy->index_count = static_cast<GLsizei>(mesh.indices->GetSize());
    proxy->vertex_offset = static_cast<GLint>(vertex_offset);
    proxy->index_offset = static_cast<GLint>(index_offset);

    memory::CopyMemory(memory + update_context.current_staging_vertex_offset,
                       mesh.vertices->GetData(), new_vertex_size);
    memory::CopyMemory(memory + update_context.current_staging_index_offset,
                       mesh.indices->GetData(), new_index_size);

    update_context.vertex_copy_regions.EmplaceLast(
        update_context.current_staging_vertex_offset,
        static_cast<GLsizeiptr>(vertex_offset *
                                sizeof(geometry::SkinnedVertex)),
        new_vertex_size);
    update_context.index_copy_regions.EmplaceLast(
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
    if (!geometry.mesh_handle) {
      COMET_LOG_WARNING(LoggerType::Rendering,
                        "MeshHandler::DestroyMeshProxies",
                        "invalid mesh handle");
      continue;
    }

    const auto proxy_handle{proxies_.TryGetHandle(geometry.mesh_handle)};

    if (!proxy_handle) {
      COMET_LOG_WARNING(
          LoggerType::Rendering, "MeshHandler::DestroyMeshProxies",
          "mesh proxy not found", "mesh_handle", geometry.mesh_handle);
      continue;
    }

    auto* proxy{proxies_.Get(proxy_handle)};

    if (!proxies_.Release(proxy_handle)) {
      continue;
    }

    vertex_buffer_.Release(proxy->vertex_offset, proxy->vertex_count);
    index_buffer_.Release(proxy->index_offset, proxy->index_count);

    proxies_.Remove(proxy_handle);
    allocator_.Deallocate(proxy);
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