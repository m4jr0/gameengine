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
#ifdef COMET_DEBUG_CULLING
  void DebugCull(Shader* shader);
  void DrawDebugCull(Shader* shader);
#endif  // COMET_DEBUG_CULLING

  u32 GetRenderProxyCount() const noexcept;
  u32 GetVisibleCount() const noexcept;

 private:
  static inline constexpr usize kMaxRenderProxyCount_{100000};
  static inline constexpr usize kDefaultRenderIndirectBatchCount_{128};
  static inline constexpr usize kDefaultRenderBatchGroupCount_{128};
  static inline constexpr usize kDefaultProxyCount_{512};
  static inline constexpr f32 kReuploadAllLocalDataThreshold_{.8f};

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
  void UpdateSkinningMatrices(const frame::SkinningBindings* bindings,
                              const frame::MatrixPalettes* palettes);
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
  void RegisterModelProxy(entity::EntityId model_entity_id,
                          RenderProxyId proxy_id);
  void UnregisterModelProxy(entity::EntityId model_entity_id,
                            RenderProxyId proxy_id);
  u64 GenerateRenderProxySortKey(const RenderProxy& proxy);
  void InitializeBuffers();
  void DestroyBuffers();
#ifdef COMET_DEBUG_RENDERING
  void InitializeDebugData();
  void DestroyDebugData();
#endif  // COMET_DEBUG_RENDERING
#ifdef COMET_DEBUG_CULLING
  void InitializeCullingDebug();
  void DestroyCullingDebug();
#endif  // COMET_DEBUG_CULLING

  FrameCount update_frame_{kInvalidFrameCount};
  usize render_proxy_count_{0};
  usize render_proxy_visible_count_{0};

  RenderProxy proxies_[kMaxRenderProxyCount_]{};

  memory::FiberFreeListAllocator proxy_local_data_allocator_{
      sizeof(GpuRenderProxyLocalData) * 16, kDefaultProxyCount_,
      memory::kEngineMemoryTagRendering};

  memory::FiberFreeListAllocator general_allocator_{
      sizeof(RenderBatchEntry) * 16, kDefaultProxyCount_,
      memory::kEngineMemoryTagRendering};

  Map<entity::EntityId, RenderProxyId> entity_id_to_proxy_id_map_{};
  Map<entity::EntityId, RenderProxyModelBindings> model_to_proxies_map_{};
  Array<entity::EntityId> proxy_id_to_entity_id_map_{};
  Array<GpuRenderProxyLocalData> proxy_local_datas_{};
  Array<RenderBatchEntry> new_batch_entries_{};
  Array<RenderBatchEntry> batch_entries_{};

  StorageHandle staging_ssbo_proxy_local_datas_handle_{kInvalidStorageHandle};
  StorageHandle ssbo_proxy_local_datas_handle_{kInvalidStorageHandle};
  StorageHandle ssbo_indirect_proxies_handle_{kInvalidStorageHandle};
  StorageHandle ssbo_proxy_ids_handle_{kInvalidStorageHandle};
  StorageHandle ssbo_proxy_instances_handle_{kInvalidStorageHandle};
  StorageHandle ssbo_matrix_palettes_handle_{kInvalidStorageHandle};
  StorageHandle ssbo_word_indices_handle_{kInvalidStorageHandle};

#ifdef COMET_DEBUG_RENDERING
  StorageHandle ssbo_debug_data_handle_{kInvalidStorageHandle};
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  StorageHandle ssbo_debug_aabbs_handle_{kInvalidStorageHandle};
  StorageHandle ssbo_debug_lines_handle_{kInvalidStorageHandle};
#endif  // COMET_DEBUG_CULLING

  GLsizei staging_ssbo_proxy_local_datas_buffer_size_{0};
  GLsizei ssbo_proxy_local_datas_buffer_size_{0};
  GLsizei ssbo_indirect_proxies_buffer_size_{0};
  GLsizei ssbo_proxy_ids_buffer_size_{0};
  GLsizei ssbo_proxy_instances_buffer_size_{0};
  GLsizei ssbo_matrix_palettes_buffer_size_{0};
  GLsizei ssbo_word_indices_buffer_size_{0};

#ifdef COMET_DEBUG_RENDERING
  GLsizei ssbo_debug_data_buffer_size_{0};
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  GLsizei ssbo_debug_aabbs_buffer_size_{0};
  GLsizei ssbo_debug_lines_buffer_size_{0};
#endif  // COMET_DEBUG_CULLING

  ShaderStoragesUpdate storages_update_{};

#ifdef COMET_DEBUG_RENDERING
  GpuDebugData* debug_data_{nullptr};
#endif  // COMET_DEBUG_RENDERING
  Shader* sparse_upload_shader_{nullptr};
  MaterialHandler* material_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  ShaderHandler* shader_handler_{nullptr};

  frame::FrameArray<RenderIndirectBatch>* indirect_batches_{nullptr};
  frame::FrameArray<RenderBatchGroup>* batch_groups_{nullptr};
  frame::FrameArray<RenderProxy>* destroyed_proxies_{nullptr};
  frame::FrameArray<RenderBatchEntry>* destroyed_batch_entries_{nullptr};
  frame::FrameOrderedSet<entity::EntityId>* destroyed_entity_ids_{nullptr};
  frame::FrameOrderedSet<RenderProxyId>* pending_proxy_ids_{nullptr};
  frame::FrameArray<usize>* pending_proxy_indices_{nullptr};
  frame::FrameArray<GpuRenderProxyLocalData>* pending_proxy_local_data_{
      nullptr};
};

}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_PROXY_HANDLER_H_
