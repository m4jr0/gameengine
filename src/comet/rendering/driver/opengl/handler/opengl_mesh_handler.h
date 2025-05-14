// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MESH_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MESH_HANDLER_H_

#include "glad/glad.h"

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/core/type/map.h"
#include "comet/geometry/geometry_common.h"
#include "comet/rendering/driver/opengl/data/opengl_mesh.h"
#include "comet/rendering/driver/opengl/data/opengl_region_gpu_buffer.h"
#include "comet/rendering/driver/opengl/data/opengl_storage.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"

namespace comet {
namespace rendering {
namespace gl {
namespace internal {
struct UpdateContext {
  GLsizei new_vertex_size{0};
  GLsizei new_index_size{0};
  GLsizei dirty_vertex_size{0};
  GLsizei dirty_index_size{0};
  GLsizei total_vertex_size{0};
  GLsizei total_index_size{0};
  GLsizei current_staging_vertex_offset{0};
  GLsizei current_staging_index_offset{0};
  frame::FrameArray<GpuBufferCopyRegion> vertex_copy_regions{};
  frame::FrameArray<GpuBufferCopyRegion> index_copy_regions{};
  void* staging_buffer{nullptr};
};

template <typename T>
struct GpuBuffer {};
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

  StorageHandle GetVertexBufferHandle() const;
  StorageHandle GetIndexBufferHandle() const;

 private:
  // Note: these are just wild guesses for now.
  static inline constexpr usize kVertexCountPerBlock_{1024};
  static inline constexpr usize kIndexCountPerBlock_{3 * kVertexCountPerBlock_};
  static inline constexpr usize kDefaultVertexCount_{
      memory::RoundUpToMultiple(3'000'000, kVertexCountPerBlock_)};
  static inline constexpr usize kDefaultIndexCount_{memory::RoundUpToMultiple(
      kIndexCountPerBlock_, 3 * kDefaultVertexCount_)};
  static inline constexpr usize kDefaultStagingBufferSize_{
      kDefaultVertexCount_ * sizeof(geometry::SkinnedVertex) +
      kDefaultIndexCount_ * sizeof(geometry::Index)};

  internal::UpdateContext PrepareUpdate(const frame::FramePacket* packet);
  void FinishUpdate(internal::UpdateContext& update_context);
  void AddMeshProxies(const frame::AddedGeometries* geometry,
                      internal::UpdateContext& update_context);
  void UpdateMeshProxies(const frame::DirtyMeshes* meshes,
                         internal::UpdateContext& update_context);
  void DestroyMeshProxies(const frame::RemovedGeometries* geometry);
  void UploadMeshProxies(const internal::UpdateContext& update_context);

  constexpr static usize kDefaultProxyCount_{4096};
  constexpr static usize kDefaultReleaseBarrierCount_{2};
  constexpr static usize kDefaultAcquireBarrierCount_{2};

  memory::FiberFreeListAllocator allocator_{
      sizeof(u32), sizeof(u32) * kDefaultProxyCount_ * 64,
      memory::kEngineMemoryTagRendering};
  Map<geometry::MeshId, usize> mesh_to_proxy_map_{};
  Array<MeshProxy> proxies_{};
  VertexGpuBuffer vertex_buffer_{};
  IndexGpuBuffer index_buffer_{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_MESH_HANDLER_H_
