// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_PROXY_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_PROXY_HANDLER_H_

#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/entity/entity_id.h"
#include "comet/rendering/driver/vulkan/data/vulkan_render_proxy.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader_data.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_material_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_mesh_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_handler.h"

namespace comet {
namespace rendering {
namespace vk {
struct RenderProxyHandlerDescr : HandlerDescr {
  MaterialHandler* material_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
  ShaderHandler* shader_handler{nullptr};
};

class RenderProxyHandler : public Handler {
 public:
  RenderProxyHandler() = delete;
  explicit RenderProxyHandler(const RenderProxyHandlerDescr& descr);
  RenderProxyHandler(const RenderProxyHandler&) = delete;
  RenderProxyHandler(RenderProxyHandler&&) = delete;
  RenderProxyHandler& operator=(const RenderProxyHandler&) = delete;
  RenderProxyHandler& operator=(RenderProxyHandler&&) = delete;
  virtual ~RenderProxyHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  void Update(frame::FramePacket* packet);
  void Reset();
  void Cull(Shader* shader);
  void Draw(Shader* shader);
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
  static inline constexpr usize kDefaultUpdateBarrierCapacity_{4};
  static inline constexpr usize kDefaultCullBarrierCapacity_{2};
#ifdef COMET_DEBUG_RENDERING
  static inline constexpr usize kDebugDataBufferCount_{2};
  static inline constexpr usize kDefaultDebugDataBarrierCapacity_{1};
#endif  // COMET_DEBUG_RENDERING
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
  void UpdateSkinningMatrices(const frame::SkinningBindings* bindings,
                              const frame::MatrixPalettes* palettes);
  void GenerateBatchEntries();
  void GenerateIndirectBatches();
  void GenerateBatchGroups();
  void UploadRenderProxyLocalData();
  void UploadAllRenderProxyLocalData();
  void UploadPendingRenderProxyLocalData();
  void CommitUpdate(frame::FramePacket* packet);
  void PrepareRenderProxyDrawData();
  void ReallocateRenderProxyDrawBuffers();
  void PopulateRenderProxyDrawData();
  void UploadRenderDrawData();
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
  usize GetDebugDataReadIndex() const;
  usize GetDebugDataWriteIndex() const;
#endif  // COMET_DEBUG_RENDERING
#ifdef COMET_DEBUG_CULLING
  void InitializeCullingDebug();
  void DestroyCullingDebug();
#endif  // COMET_DEBUG_CULLING

  FrameIndex update_frame_{kInvalidFrameIndex};
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

  Buffer staging_ssbo_proxy_local_datas_{};
  Buffer ssbo_proxy_local_datas_{};
  Buffer staging_ssbo_indirect_proxies_{};
  Buffer ssbo_indirect_proxies_{};
  Buffer ssbo_proxy_ids_{};
  Buffer staging_ssbo_proxy_instances_{};
  Buffer ssbo_proxy_instances_{};
  Buffer ssbo_matrix_palettes_{};
  Buffer ssbo_word_indices_{};

#ifdef COMET_DEBUG_RENDERING
  Buffer ssbo_debug_data_[kDebugDataBufferCount_]{};
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  Buffer ssbo_debug_aabbs_{};
  Buffer ssbo_debug_lines_{};
#endif  // COMET_DEBUG_CULLING

  ShaderStoragesUpdate storages_update_{};

#ifdef COMET_DEBUG_RENDERING
  GpuDebugData* debug_data_[kDebugDataBufferCount_]{nullptr, nullptr};
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
  frame::FrameArray<VkBufferMemoryBarrier>* post_update_barriers_{nullptr};
  frame::FrameArray<VkBufferMemoryBarrier>* cull_barriers_{nullptr};
  frame::FrameArray<VkBufferMemoryBarrier>* shader_to_transfer_barriers_{
      nullptr};
#ifdef COMET_DEBUG_RENDERING
  frame::FrameArray<VkBufferMemoryBarrier>* debug_data_barriers_{nullptr};
#endif  // COMET_DEBUG_RENDERING
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_PROXY_HANDLER_H_
