// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PROXY_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PROXY_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_container.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/entity/type/entity_id.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_material_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_mesh_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_render_proxy.h"

namespace comet {
namespace rendering {
namespace vk {
struct RenderProxySparseUploadData {
  VkBuffer ssbo_word_indices_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_word_indices_size{0};

  VkBuffer ssbo_source_words_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_source_words_size{0};

  VkBuffer ssbo_destination_words_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_destination_words_size{0};

  u32 word_count{0};
};

struct RenderProxyGpuData {
  VkBuffer ssbo_proxy_local_datas_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_proxy_ids_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_proxy_instances_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_indirect_proxies_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_matrix_palettes_handle{VK_NULL_HANDLE};
#ifdef COMET_DEBUG_RENDERING
  VkBuffer ssbo_debug_data_handle{VK_NULL_HANDLE};
#endif  // COMET_DEBUG_RENDERING
#ifdef COMET_DEBUG_CULLING
  VkBuffer ssbo_debug_aabbs_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_debug_lines_handle{VK_NULL_HANDLE};
#endif  // COMET_DEBUG_CULLING
  VkDeviceSize ssbo_proxy_local_datas_size{0};
  VkDeviceSize ssbo_proxy_ids_size{0};
  VkDeviceSize ssbo_proxy_instances_size{0};
  VkDeviceSize ssbo_indirect_proxies_size{0};
  VkDeviceSize ssbo_matrix_palettes_size{0};
#ifdef COMET_DEBUG_RENDERING
  VkDeviceSize ssbo_debug_data_size{0};
#endif  // COMET_DEBUG_RENDERING
#ifdef COMET_DEBUG_CULLING
  VkDeviceSize ssbo_debug_aabbs_size{0};
  VkDeviceSize ssbo_debug_lines_size{0};
#endif  // COMET_DEBUG_CULLING
};

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
  ~RenderProxyHandler() override = default;

  void Update(frame::FramePacket* packet);
  void Reset();

  u32 GetRenderProxyCount() const noexcept;
  u32 GetVisibleCount() const noexcept;
  RenderProxyGpuData GetGpuData(FrameInFlightIndex frame_index) const noexcept;

  bool HasPendingSparseUpload() const noexcept;
  u32 GetSparseUploadWordCount() const noexcept;
  u32 GetSparseUploadGroupCount() const noexcept;
  RenderProxySparseUploadData GetSparseUploadGpuData() const noexcept;
  void PopulateSparseUploadBarriers(Array<VkBufferMemoryBarrier>& out) const;

  u32 GetCullGroupCount() const noexcept;
  void PopulateDrawCullBarriers(FrameInFlightIndex frame_index,
                                Array<VkBufferMemoryBarrier>& out) const;
#ifdef COMET_DEBUG_RENDERING
  void PopulateCullDebugReadBarriers(FrameInFlightIndex frame_index,
                                     Array<VkBufferMemoryBarrier>& out) const;

  void PrepareCullDebugWrite(FrameInFlightIndex frame_index);
#endif  // COMET_DEBUG_RENDERING

  const Array<RenderBatchGroup>* GetBatchGroups() const noexcept;
  const Array<RenderIndirectBatch>* GetIndirectBatches() const noexcept;
  const Buffer& GetIndirectBuffer(
      FrameInFlightIndex frame_index) const noexcept;
  const Buffer& GetShadowIndirectBuffer(
      FrameInFlightIndex frame_index) const noexcept;

#ifdef COMET_DEBUG_CULLING
  const Buffer& GetDebugLineBuffer() const noexcept;
  u32 GetDebugLineVertexCount() const noexcept;
#endif  // COMET_DEBUG_CULLING

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  static inline constexpr usize kMaxRenderProxyCount_{100000};
  static inline constexpr usize kDefaultRenderIndirectBatchCount_{128};
  static inline constexpr usize kDefaultRenderBatchGroupCount_{128};
  static inline constexpr usize kDefaultProxyCount_{512};
  static inline constexpr f32 kReuploadAllLocalDataThreshold_{.8f};
  static inline constexpr usize kDefaultUpdateBarrierCapacity_{4};
  static inline constexpr usize kDefaultCullBarrierCapacity_{2};
  static inline constexpr usize kDefaultSparseUploadBarrierCapacity_{1};
#ifdef COMET_DEBUG_RENDERING
  static inline constexpr usize kDebugDataBufferCount_{2};
  static inline constexpr usize kDefaultDebugDataBarrierCapacity_{1};
#endif  // COMET_DEBUG_RENDERING
  static inline constexpr usize kDefaultShaderToTransferBarrierCapacity_{1};

  static bool OnRenderBatchSort(const RenderBatchEntry& a,
                                const RenderBatchEntry& b);
  static u64 GenerateRenderProxySortKey(const RenderProxy& proxy);

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

  void PrepareRenderProxyDrawData(FrameInFlightIndex frame_index);
  void ReallocateRenderProxyDrawBuffers(FrameInFlightIndex frame_index);
  void PopulateRenderProxyDrawData(FrameInFlightIndex frame_index);

  void UploadRenderDrawData(FrameInFlightIndex frame_index);
  void PopulateRenderIndirectProxy(BatchId batch_id,
                                   GpuIndirectRenderProxy* memory);
  void PopulateShadowRenderIndirectProxy(BatchId batch_id,
                                         GpuIndirectRenderProxy* memory);
  void PopulateProxyInstances(BatchId batch_id, GpuRenderProxyInstance* memory,
                              usize& proxy_instance_index);

  void RegisterModelProxy(entity::EntityId model_entity_id,
                          RenderProxyId proxy_id);
  void UnregisterModelProxy(entity::EntityId model_entity_id,
                            RenderProxyId proxy_id);

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

  void DestroyLiveProxyMaterials();

  FrameIndex update_frame_{kInvalidFrameIndex};
  u32 render_proxy_count_{0};
  u32 render_proxy_visible_count_{0};
  u32 sparse_upload_word_count_{0};

  RenderProxy proxies_[kMaxRenderProxyCount_]{};

  memory::FiberFreeListAllocator proxy_local_data_allocator_{
      sizeof(GpuRenderProxyLocalData) * 16, kDefaultProxyCount_,
      memory::kEngineMemoryTagRendering};

  memory::FiberFreeListAllocator general_allocator_{
      sizeof(RenderBatchEntry) * 16, kDefaultProxyCount_,
      memory::kEngineMemoryTagRendering};

  memory::PlatformAllocator platform_allocator_{
      memory::kEngineMemoryTagRendering};

  Map<entity::EntityId, RenderProxyId> entity_id_to_proxy_id_map_{};
  Map<entity::EntityId, RenderProxyModelBindings> model_to_proxies_map_{};
  Array<entity::EntityId> proxy_id_to_entity_id_map_{};
  Array<GpuRenderProxyLocalData> proxy_local_datas_{};
  Array<RenderBatchEntry> new_batch_entries_{};
  Array<RenderBatchEntry> batch_entries_{};

  Buffer staging_ssbo_proxy_local_datas_{};
  Buffer ssbo_proxy_local_datas_{};
  Buffer ssbo_proxy_ids_{};
  Buffer ssbo_matrix_palettes_{};
  Buffer ssbo_word_indices_{};

  Array<Buffer> staging_ssbo_indirect_proxies_{};
  Array<Buffer> ssbo_indirect_proxies_{};
  Array<Buffer> staging_ssbo_shadow_indirect_proxies_{};
  Array<Buffer> ssbo_shadow_indirect_proxies_{};
  Array<Buffer> staging_ssbo_proxy_instances_{};
  Array<Buffer> ssbo_proxy_instances_{};

#ifdef COMET_DEBUG_RENDERING
  Buffer ssbo_debug_data_[kDebugDataBufferCount_]{};
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  Buffer ssbo_debug_aabbs_{};
  Buffer ssbo_debug_lines_{};
#endif  // COMET_DEBUG_CULLING

#ifdef COMET_DEBUG_RENDERING
  GpuDebugData* debug_data_[kDebugDataBufferCount_]{nullptr, nullptr};
#endif  // COMET_DEBUG_RENDERING
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
  frame::FrameArray<VkBufferMemoryBarrier>* shader_to_transfer_barriers_{
      nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PROXY_HANDLER_H_
