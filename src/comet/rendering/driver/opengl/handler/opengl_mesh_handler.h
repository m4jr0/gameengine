// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MESH_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MESH_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/geometry/geometry_common.h"
#include "comet/rendering/driver/opengl/data/opengl_mesh.h"
#include "comet/rendering/driver/opengl/data/opengl_storage.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "glad/glad.h"

namespace comet {
namespace rendering {
namespace gl {
namespace internal {
typedef struct CopyRegion {
  GLsizeiptr src_offset;
  GLsizeiptr dst_offset;
  GLsizeiptr size;
} CopyRegion;

struct UpdateContext {
  GLsizeiptr new_vertex_size{0};
  GLsizeiptr new_index_size{0};
  GLsizeiptr dirty_vertex_size{0};
  GLsizeiptr dirty_index_size{0};
  GLsizeiptr total_vertex_size{0};
  GLsizeiptr total_index_size{0};
  GLsizeiptr current_staging_vertex_offset{0};
  GLsizeiptr current_staging_index_offset{0};
  frame::FrameArray<CopyRegion> vertex_copy_regions{};
  frame::FrameArray<CopyRegion> index_copy_regions{};
  void* staging_buffer{nullptr};
};

struct FreeRegion {
  static inline constexpr auto kInvalidOffset{static_cast<u32>(-1)};

  u32 offset{0};
  GLsizeiptr size{0};
};
}  // namespace internal

using MeshHandlerDescr = HandlerDescr;

class MeshHandler : public Handler {
 public:
  MeshHandler() = delete;
  explicit MeshHandler(const MeshHandlerDescr& descr);
  MeshHandler(const MeshHandler&) = delete;
  MeshHandler(MeshHandler&&) = delete;
  MeshHandler& operator=(const MeshHandler&) = delete;
  MeshHandler& operator=(MeshHandler&&) = delete;
  virtual ~MeshHandler() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update(const frame::FramePacket* packet);
  void Bind();

  MeshProxyHandle GetHandle(geometry::MeshId mesh_id) const;
  const MeshProxy* Get(MeshProxyHandle handle) const;

 private:
  static inline constexpr usize kDefaultStagingBufferSize_{
      67108864};  // 64 MiB.

  static u32 AllocateFromFreeList(Array<internal::FreeRegion>& free_list,
                                  GLsizeiptr size);
  static void FreeToFreeList(Array<internal::FreeRegion>& free_list, u32 offset,
                             GLsizeiptr size);

  internal::UpdateContext PrepareUpdate(const frame::FramePacket* packet);
  void FinishUpdate(internal::UpdateContext& update_context);
  void AddMeshProxies(const frame::AddedGeometries* geometry,
                      internal::UpdateContext& update_context);
  void UpdateMeshProxies(const frame::DirtyMeshes* meshes,
                         internal::UpdateContext& update_context);
  void DestroyMeshProxies(const frame::RemovedGeometries* geometry);
  void UploadMeshProxies(const internal::UpdateContext& update_context);
  void ClearAllMeshProxies();
  void ResizeVertexBuffer(GLsizeiptr new_size);
  void ResizeIndexBuffer(GLsizeiptr new_size);

  constexpr static usize kDefaultProxyCount_{4096};
  constexpr static usize kDefaultReleaseBarrierCount_{2};
  constexpr static usize kDefaultAcquireBarrierCount_{2};

  memory::FiberFreeListAllocator allocator_{
      sizeof(u32), sizeof(u32) * kDefaultProxyCount_ * 64,
      memory::kEngineMemoryTagRendering};
  Map<geometry::MeshId, usize> mesh_to_proxy_map_{};
  Array<MeshProxy> proxies_{};
  Array<internal::FreeRegion> free_vertex_regions_{};
  Array<internal::FreeRegion> free_index_regions_{};
  GLsizeiptr uploaded_vertex_buffer_size_{0};
  GLsizeiptr uploaded_index_buffer_size_{0};
  StorageHandle uploaded_vertex_buffer_handle_{kInvalidStorageHandle};
  StorageHandle uploaded_index_buffer_handle_{kInvalidStorageHandle};
  void* uploaded_vertex_buffer_memory_{nullptr};
  void* uploaded_index_buffer_memory_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MESH_HANDLER_H_
