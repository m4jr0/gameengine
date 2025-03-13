// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_PROXY_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_PROXY_HANDLER_H_

#include "glad/glad.h"

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/opengl/data/opengl_render_proxy.h"
#include "comet/rendering/driver/opengl/data/opengl_shader.h"
#include "comet/rendering/driver/opengl/data/opengl_storage.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_material_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_mesh_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_shader_handler.h"

namespace comet {
namespace rendering {
namespace gl {
struct RenderProxyHandlerDescr : HandlerDescr {
  MaterialHandler* material_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
  ShaderHandler* shader_handler{nullptr};
};

class RenderProxyHandler : public Handler {
 public:
  RenderProxyHandler(const RenderProxyHandlerDescr& descr)
      : Handler{descr},
        material_handler_{descr.material_handler},
        mesh_handler_{descr.mesh_handler},
        shader_handler_{descr.shader_handler} {
    COMET_ASSERT(material_handler_ != nullptr, "Material handler is null!");
    COMET_ASSERT(mesh_handler_ != nullptr, "Mesh handler is null!");
    COMET_ASSERT(shader_handler_ != nullptr, "Shader handler is null!");
  }

  void Initialize() override;
  void Shutdown() override;

  void Update(frame::FramePacket* packet);
  void Cull(Shader* shader);
  void Draw(Shader* shader, FrameCount frame_count);
  u32 GetRenderProxyCount() const noexcept;

 private:
  static inline constexpr usize kMaxRenderProxyCount_{100000};
  static inline constexpr usize kDefaultRenderIndirectBatchCount_{128};
  static inline constexpr usize kDefaultRenderBatchGroupCount_{128};
  static inline constexpr usize kDefaultProxyCount_{512};
  static inline constexpr f32 kReuploadAllLocalDataThreshold_{.8f};
  static inline constexpr usize kDefaultUpdateBarrierCapacity_{4};
  static inline constexpr usize kDefaultCullBarrierCapacity_{2};
  static inline constexpr usize kDefaultShaderToTransferBarrierCapacity_{1};

  static bool OnRenderBatchSort(const RenderBatchEntry& a,
                                const RenderBatchEntry& b);

  void GenerateUpdateTemporaryStructures(const frame::FramePacket* packet);
  void DestroyUpdateTemporaryStructures();
  void ApplyRenderProxyChanges(const frame::FramePacket* packet);
  void ProcessBatches();
  void GenerateRenderProxies(const frame::AddedGeometries* geometries);
  void UpdateRenderProxies(const frame::DirtyMeshes* meshes,
                           const frame::DirtyTransforms* transforms);
  void DestroyRenderProxies(const frame::RemovedGeometries* geometries);
  void GenerateBatchEntries();
  void GenerateIndirectBatches();
  void GenerateBatchGroups();

  void UploadRenderProxyLocalData();
  void UploadAllRenderProxyLocalData();
  void UploadPendingRenderProxyLocalData();
  void CommitUpdate(frame::FramePacket* packet);
  void ReallocateRenderProxyDrawBuffers();
  void PrepareRenderProxyDrawData();
  void PopulateRenderProxyDrawData();
  void PopulateRenderIndirectProxy(BatchId batch_id,
                                   GpuIndirectRenderProxy* memory);
  void PopulateProxyInstances(BatchId batch_id, GpuRenderProxyInstance* memory,
                              usize& proxy_instance_index);
  u64 GenerateRenderProxySortKey(const RenderProxy& proxy);

  FrameCount update_frame_{kInvalidFrameCount};
  usize render_proxy_count_{0};

  RenderProxy proxies_[kMaxRenderProxyCount_]{};

  memory::FiberFreeListAllocator proxy_local_data_allocator_{
      sizeof(GpuRenderProxyLocalData) * 16, kDefaultProxyCount_,
      memory::kEngineMemoryTagRendering};

  memory::FiberFreeListAllocator general_allocator_{
      sizeof(RenderBatchEntry) * 16, kDefaultProxyCount_,
      memory::kEngineMemoryTagRendering};

  Map<entity::EntityId, RenderProxyId> entity_id_to_proxy_id_map_{};
  Array<entity::EntityId> proxy_id_to_entity_id_map_{};
  Array<GpuRenderProxyLocalData> proxy_local_datas_{};
  Array<RenderBatchEntry> new_batch_entries_{};
  Array<RenderBatchEntry> batch_entries_{};

  StorageHandle ssbo_indirect_proxies_handle_{kInvalidStorageHandle};
  StorageHandle ssbo_proxy_instances_handle_{kInvalidStorageHandle};
  StorageHandle ssbo_proxy_local_datas_handle_{kInvalidStorageHandle};
  StorageHandle ssbo_proxy_ids_handle_{kInvalidStorageHandle};
  StorageHandle ssbo_word_indices_handle_{kInvalidStorageHandle};

  GLsizei ssbo_indirect_proxies_buffer_size_{0};
  GLsizei ssbo_proxy_instances_buffer_size_{0};
  GLsizei ssbo_proxy_local_datas_buffer_size_{0};
  GLsizei ssbo_proxy_ids_buffer_size_{0};
  GLsizei ssbo_word_indices_buffer_size_{0};

  ShaderStoragesUpdate storages_update_{};

  Shader* sparse_upload_shader_{nullptr};
  MaterialHandler* material_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  ShaderHandler* shader_handler_{nullptr};

  frame::FrameArray<RenderIndirectBatch>* indirect_batches_{nullptr};
  frame::FrameArray<RenderBatchGroup>* batch_groups_{nullptr};
  frame::FrameArray<RenderProxy>* destroyed_proxies_{nullptr};
  frame::FrameArray<RenderBatchEntry>* destroyed_batch_entries_{nullptr};
  frame::FrameOrderedSet<RenderProxyId>* pending_proxy_ids_{nullptr};
  frame::FrameArray<usize>* pending_proxy_indices_{nullptr};
  frame::FrameArray<GpuRenderProxyLocalData>* pending_proxy_local_data_{
      nullptr};
};

}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_PROXY_HANDLER_H_
