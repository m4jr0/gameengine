// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_MESH_HANDLER_H_
#define COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_MESH_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/runtime/frame/frame_container.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/memory/allocator/free_list_allocator.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/runtime/geometry/mesh.h"
#include "comet/core/container/array.h"
#include "comet/runtime/shared_instance_registry.h"
#include "comet/data/geometry/mesh.h"
#include "comet/render/driver/opengl/handler/opengl_handler.h"
#include "comet/render/driver/opengl/type/opengl_mesh.h"
#include "comet/render/driver/opengl/type/opengl_region_gpu_buffer.h"
#include "comet/render/driver/opengl/type/opengl_storage.h"

namespace comet {
namespace render {
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
  ~MeshHandler() override = default;

  void Update(const frame::FramePacket* packet);

  const MeshProxy* Get(geometry::MeshHandle handle) const;
  const MeshProxy* TryGet(geometry::MeshHandle handle) const;

  GlNativeStorageHandle GetVertexBufferHandle() const;
  GlNativeStorageHandle GetIndexBufferHandle() const;
  ShaderVertexSource GetVertexSource() const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  struct MeshProxyRegistryTag;
  using MeshProxyRegistry =
      SharedInstanceRegistry<geometry::MeshHandle, MeshProxyRegistryTag,
                             MeshProxy>;

  static inline constexpr usize kVertexCountPerBlock_{2048};
  static inline constexpr usize kIndexCountPerBlock_{3 * kVertexCountPerBlock_};
  static inline constexpr usize kDefaultVertexCount_{
      memory::RoundUpToMultiple(3'000'000, kVertexCountPerBlock_)};
  static inline constexpr usize kDefaultIndexCount_{memory::RoundUpToMultiple(
      3 * kDefaultVertexCount_, kIndexCountPerBlock_)};
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

  static constexpr usize kDefaultProxyCount_{4096};

  memory::FiberFreeListAllocator allocator_{
      sizeof(MeshProxy), sizeof(MeshProxy) * kDefaultProxyCount_,
      kEngineMemoryTagRender};

  memory::PlatformAllocator registry_allocator_{
      kEngineMemoryTagRender};

  MeshProxyRegistry proxies_{};
  VertexGpuBuffer vertex_buffer_{};
  IndexGpuBuffer index_buffer_{};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_MESH_HANDLER_H_