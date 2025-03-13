// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "opengl_mesh_handler.h"

#include <utility>

#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/opengl_debug.h"

namespace comet {
namespace rendering {
namespace gl {
MeshHandler::MeshHandler(const MeshHandlerDescr& descr) : Handler{descr} {}

void MeshHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();
  mesh_to_proxy_map_ = {&allocator_, kDefaultProxyCount_};
  proxies_ = {&allocator_, kDefaultProxyCount_};
  free_vertex_regions_ = {&allocator_, kDefaultProxyCount_};
  free_index_regions_ = {&allocator_, kDefaultProxyCount_};
  ResizeVertexBuffer(kDefaultStagingBufferSize_);
  ResizeIndexBuffer(kDefaultStagingBufferSize_);
}

void MeshHandler::Shutdown() {
  ClearAllMeshProxies();
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
  glBindBuffer(GL_ARRAY_BUFFER, uploaded_vertex_buffer_handle_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uploaded_index_buffer_handle_);
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
  return uploaded_vertex_buffer_handle_;
}

StorageHandle MeshHandler::GetIndexBufferHandle() const {
  return uploaded_index_buffer_handle_;
}

GLint MeshHandler::AllocateFromFreeList(Array<internal::FreeRegion>& free_list,
                                        GLsizei size) {
  COMET_PROFILE("MeshHandler::AllocateFromFreeList");

  for (auto it{free_list.begin()}; it != free_list.end(); ++it) {
    if (it->size < size) {
      continue;
    }

    auto offset{it->offset};

    if (it->size > size) {
      it->offset += static_cast<GLint>(size);
      it->size -= size;
    } else {
      free_list.RemoveFromPos(it);
    }

    return offset;
  }

  return internal::FreeRegion::kInvalidOffset;
}

void MeshHandler::FreeToFreeList(Array<internal::FreeRegion>& free_list,
                                 GLint offset, GLsizei size) {
  COMET_PROFILE("MeshHandler::FreeToFreeList");
  free_list.EmplaceBack(offset, size);

  std::sort(free_list.begin(), free_list.end(),
            [](const internal::FreeRegion& a, const internal::FreeRegion& b) {
              return a.offset < b.offset;
            });

  for (usize i{0}; i < free_list.GetSize() - 1; ++i) {
    if (free_list[i].offset + free_list[i].size != free_list[i + 1].offset) {
      continue;
    }

    free_list[i].size += free_list[i + 1].size;
    free_list.RemoveFromIndex(i + 1);
    --i;
  }
}

internal::UpdateContext MeshHandler::PrepareUpdate(
    const frame::FramePacket* packet) {
  COMET_PROFILE("MeshHandler::PrepareUpdate");
  internal::UpdateContext update_context{};

  for (const auto& geometry : *packet->added_geometries) {
    update_context.new_vertex_size += static_cast<GLsizei>(
        geometry.vertices->GetSize() * sizeof(geometry::Vertex));
    update_context.new_index_size += static_cast<GLsizei>(
        geometry.indices->GetSize() * sizeof(geometry::Index));
  }

  for (const auto& mesh : *packet->dirty_meshes) {
    update_context.dirty_vertex_size += static_cast<GLsizei>(
        mesh.vertices->GetSize() * sizeof(geometry::Vertex));
    update_context.dirty_index_size +=
        static_cast<GLsizei>(mesh.indices->GetSize() * sizeof(geometry::Index));
  }

  update_context.total_vertex_size =
      update_context.new_vertex_size + update_context.dirty_vertex_size;
  update_context.total_index_size =
      update_context.new_index_size + update_context.dirty_index_size;

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
                                          sizeof(geometry::Vertex))};
    auto index_size{static_cast<GLsizei>(geometry.indices->GetSize() *
                                         sizeof(geometry::Index))};

    memory::CopyMemory(memory + update_context.current_staging_vertex_offset,
                       geometry.vertices->GetData(), vertex_size);
    memory::CopyMemory(memory + update_context.current_staging_index_offset,
                       geometry.indices->GetData(), index_size);

    auto vertex_byte_offset{
        AllocateFromFreeList(free_vertex_regions_, vertex_size)};
    auto index_byte_offset{
        AllocateFromFreeList(free_index_regions_, index_size)};

    if (vertex_byte_offset == internal::FreeRegion::kInvalidOffset) {
      ResizeVertexBuffer(math::Max(uploaded_vertex_buffer_size_ + vertex_size,
                                   uploaded_vertex_buffer_size_ * 2));
      vertex_byte_offset =
          AllocateFromFreeList(free_vertex_regions_, vertex_size);
    }

    if (index_byte_offset == internal::FreeRegion::kInvalidOffset) {
      ResizeIndexBuffer(math::Max(uploaded_index_buffer_size_ + index_size,
                                  uploaded_index_buffer_size_ * 2));
      index_byte_offset = AllocateFromFreeList(free_index_regions_, index_size);
    }

    COMET_ASSERT(vertex_byte_offset != internal::FreeRegion::kInvalidOffset,
                 "Not enough memory to upload vertices!");
    COMET_ASSERT(index_byte_offset != internal::FreeRegion::kInvalidOffset,
                 "Not enough memory to upload indices!");

    update_context.vertex_copy_regions.EmplaceBack(
        update_context.current_staging_vertex_offset, vertex_byte_offset,
        vertex_size);
    update_context.index_copy_regions.EmplaceBack(
        update_context.current_staging_index_offset, index_byte_offset,
        index_size);

    update_context.current_staging_vertex_offset += vertex_size;
    update_context.current_staging_index_offset += index_size;

    mesh_to_proxy_map_[geometry.mesh_id] = proxies_.GetSize();
    auto& proxy{proxies_.EmplaceBack()};
    proxy.vertex_count = static_cast<u32>(geometry.vertices->GetSize());
    proxy.index_count = static_cast<u32>(geometry.indices->GetSize());
    proxy.vertex_offset = vertex_byte_offset / sizeof(geometry::Vertex);
    proxy.index_offset = index_byte_offset / sizeof(geometry::Index);
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
                                              sizeof(geometry::Vertex))};
    auto new_index_size{static_cast<GLsizei>(mesh.indices->GetSize() *
                                             sizeof(geometry::Index))};
    auto vertex_byte_offset{
        static_cast<GLint>(proxy.vertex_offset * sizeof(geometry::Vertex))};
    auto index_byte_offset{
        static_cast<GLint>(proxy.index_offset * sizeof(geometry::Index))};

    if (new_vertex_size >
        static_cast<GLsizei>(proxy.vertex_count * sizeof(geometry::Vertex))) {
      FreeToFreeList(free_vertex_regions_, proxy.vertex_offset,
                     proxy.vertex_count * sizeof(geometry::Vertex));

      vertex_byte_offset =
          AllocateFromFreeList(free_vertex_regions_, new_vertex_size);

      if (vertex_byte_offset == internal::FreeRegion::kInvalidOffset) {
        ResizeVertexBuffer(
            math::Max(uploaded_vertex_buffer_size_ + new_vertex_size,
                      uploaded_vertex_buffer_size_ * 2));
        vertex_byte_offset =
            AllocateFromFreeList(free_vertex_regions_, new_vertex_size);
      }
    }

    if (new_index_size >
        static_cast<GLsizei>(proxy.index_count * sizeof(geometry::Index))) {
      FreeToFreeList(free_index_regions_, proxy.index_offset,
                     proxy.index_count * sizeof(geometry::Index));

      index_byte_offset =
          AllocateFromFreeList(free_index_regions_, new_index_size);

      if (index_byte_offset == internal::FreeRegion::kInvalidOffset) {
        ResizeIndexBuffer(
            math::Max(uploaded_index_buffer_size_ + new_index_size,
                      uploaded_index_buffer_size_ * 2));
        index_byte_offset =
            AllocateFromFreeList(free_index_regions_, new_index_size);
      }
    }

    COMET_ASSERT(vertex_byte_offset != internal::FreeRegion::kInvalidOffset,
                 "Not enough memory to upload vertices!");
    COMET_ASSERT(index_byte_offset != internal::FreeRegion::kInvalidOffset,
                 "Not enough memory to upload indices!");

    proxy.vertex_count = static_cast<u32>(mesh.vertices->GetSize());
    proxy.index_count = static_cast<u32>(mesh.indices->GetSize());
    proxy.vertex_offset =
        static_cast<u32>(vertex_byte_offset / sizeof(geometry::Vertex));
    proxy.index_offset =
        static_cast<u32>(index_byte_offset / sizeof(geometry::Index));

    memory::CopyMemory(memory + update_context.current_staging_vertex_offset,
                       mesh.vertices->GetData(), new_vertex_size);
    memory::CopyMemory(memory + update_context.current_staging_index_offset,
                       mesh.indices->GetData(), new_index_size);

    update_context.vertex_copy_regions.EmplaceBack(
        update_context.current_staging_vertex_offset, vertex_byte_offset,
        new_vertex_size);
    update_context.index_copy_regions.EmplaceBack(
        update_context.current_staging_index_offset, index_byte_offset,
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

    auto& proxy = proxies_[*proxy_id];

    FreeToFreeList(free_vertex_regions_, proxy.vertex_offset,
                   proxy.vertex_count * sizeof(geometry::Vertex));
    FreeToFreeList(free_index_regions_, proxy.index_offset,
                   proxy.index_count * sizeof(geometry::Index));
    mesh_to_proxy_map_.Remove(geometry.mesh_id);
  }
}

void MeshHandler::UploadMeshProxies(
    const internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::UploadMeshProxies");
  const auto* memory{static_cast<const u8*>(update_context.staging_buffer)};

  if (!update_context.vertex_copy_regions.IsEmpty()) {
    glBindBuffer(GL_ARRAY_BUFFER, uploaded_vertex_buffer_handle_);
    for (const auto& region : update_context.vertex_copy_regions) {
      memory::CopyMemory(
          static_cast<u8*>(uploaded_vertex_buffer_memory_) + region.dst_offset,
          memory + region.src_offset, region.size);
    }
    glBindBuffer(GL_ARRAY_BUFFER, kInvalidStorageHandle);
  }

  if (!update_context.index_copy_regions.IsEmpty()) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uploaded_index_buffer_handle_);
    for (const auto& region : update_context.index_copy_regions) {
      memory::CopyMemory(
          static_cast<u8*>(uploaded_index_buffer_memory_) + region.dst_offset,
          memory + region.src_offset, region.size);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kInvalidStorageHandle);
  }
}

void MeshHandler::ClearAllMeshProxies() {
  COMET_PROFILE("MeshHandler::ClearAllMeshProxies");
  proxies_ = {};
  mesh_to_proxy_map_ = {};
  free_vertex_regions_ = {};
  free_index_regions_ = {};

  if (uploaded_vertex_buffer_handle_ != kInvalidStorageHandle) {
    glDeleteBuffers(1, &uploaded_vertex_buffer_handle_);
    uploaded_vertex_buffer_handle_ = kInvalidStorageHandle;
    uploaded_vertex_buffer_memory_ = nullptr;
  }

  if (uploaded_index_buffer_handle_ != kInvalidStorageHandle) {
    glDeleteBuffers(1, &uploaded_index_buffer_handle_);
    uploaded_index_buffer_handle_ = kInvalidStorageHandle;
    uploaded_index_buffer_memory_ = nullptr;
  }
}

void MeshHandler::ResizeVertexBuffer(GLsizei new_size) {
  COMET_PROFILE("MeshHandler::ResizeVertexBuffer");

  if (new_size <= uploaded_vertex_buffer_size_) {
    return;
  }

  auto old_capacity{uploaded_vertex_buffer_size_};

  if (uploaded_vertex_buffer_handle_ != kInvalidStorageHandle) {
    glBindBuffer(GL_ARRAY_BUFFER, uploaded_vertex_buffer_handle_);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, kInvalidStorageHandle);
    glDeleteBuffers(1, &uploaded_vertex_buffer_handle_);
  }

  glGenBuffers(1, &uploaded_vertex_buffer_handle_);
  glBindBuffer(GL_ARRAY_BUFFER, uploaded_vertex_buffer_handle_);
  COMET_GL_SET_UNIFORM_BUFFER_DEBUG_LABEL(uploaded_vertex_buffer_handle_,
                                          "uploaded_vertex_buffer_handle_");
  glBufferStorage(
      GL_ARRAY_BUFFER, new_size, nullptr,
      GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  uploaded_vertex_buffer_memory_ = glMapBufferRange(
      GL_ARRAY_BUFFER, 0, new_size,
      GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  glBindBuffer(GL_ARRAY_BUFFER, kInvalidStorageHandle);

  uploaded_vertex_buffer_size_ = new_size;
  free_vertex_regions_.EmplaceBack(static_cast<GLint>(old_capacity),
                                   new_size - old_capacity);
}

void MeshHandler::ResizeIndexBuffer(GLsizei new_size) {
  COMET_PROFILE("MeshHandler::ResizeIndexBuffer");

  if (new_size <= uploaded_index_buffer_size_) {
    return;
  }

  auto old_capacity{uploaded_index_buffer_size_};

  if (uploaded_index_buffer_handle_ != kInvalidStorageHandle) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uploaded_index_buffer_handle_);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kInvalidStorageHandle);
    glDeleteBuffers(1, &uploaded_index_buffer_handle_);
  }

  glGenBuffers(1, &uploaded_index_buffer_handle_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uploaded_index_buffer_handle_);
  COMET_GL_SET_UNIFORM_BUFFER_DEBUG_LABEL(uploaded_index_buffer_handle_,
                                          "uploaded_index_buffer_handle_");
  glBufferStorage(
      GL_ELEMENT_ARRAY_BUFFER, new_size, nullptr,
      GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  uploaded_index_buffer_memory_ = glMapBufferRange(
      GL_ELEMENT_ARRAY_BUFFER, 0, new_size,
      GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kInvalidStorageHandle);

  uploaded_index_buffer_size_ = new_size;
  free_index_regions_.EmplaceBack(static_cast<GLint>(old_capacity),
                                  new_size - old_capacity);
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
