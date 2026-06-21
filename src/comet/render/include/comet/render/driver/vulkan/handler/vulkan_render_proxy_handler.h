// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PROXY_HANDLER_H_
#define COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PROXY_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/runtime/frame/frame_container.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/memory/allocator/free_list_allocator.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/memory/memory_tag.h"
#include "comet/core/container/array.h"
#include "comet/core/math/matrix.h"
#include "comet/render/driver/vulkan/handler/vulkan_material_handler.h"
#include "comet/render/driver/vulkan/handler/vulkan_mesh_handler.h"
#include "comet/render/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/render/driver/vulkan/type/vulkan_render_proxy.h"
#include "comet/render/driver/vulkan/type/vulkan_shadow.h"
#include "comet/render/render_proxy_record_store.h"
#include "comet/render/type/render_proxy.h"

namespace comet {
namespace render {
namespace vk {
struct RenderProxySparseUploadData {
  VkBuffer ssbo_sparse_upload_word_indices_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_sparse_upload_word_indices_size{0};

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

  VkDeviceSize ssbo_proxy_local_datas_size{0};
  VkDeviceSize ssbo_proxy_ids_size{0};
  VkDeviceSize ssbo_proxy_instances_size{0};
  VkDeviceSize ssbo_indirect_proxies_size{0};
  VkDeviceSize ssbo_matrix_palettes_size{0};
};

struct RenderProxyHandlerDescr : HandlerDescr {
  const RenderProxyRecordStore* render_proxy_record_store{nullptr};
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
  u32 GetRenderProxyInstanceCount() const noexcept;

  MaterialHandle GetMaterialHandle(RenderProxyId proxy_id) const;

  RenderProxyGpuData GetGpuData(FrameInFlightIndex frame_index) const noexcept;

  bool HasPendingSparseUpload(FrameInFlightIndex frame_index) const noexcept;
  u32 GetSparseUploadWordCount(FrameInFlightIndex frame_index) const noexcept;
  u32 GetSparseUploadGroupCount(FrameInFlightIndex frame_index) const noexcept;
  RenderProxySparseUploadData GetSparseUploadGpuData(
      FrameInFlightIndex frame_index) const noexcept;
  void PopulateSparseUploadReadBarriers(
      FrameInFlightIndex frame_index, Array<VkBufferMemoryBarrier>& out) const;
  void PopulateSparseUploadBarriers(FrameInFlightIndex frame_index,
                                    Array<VkBufferMemoryBarrier>& out) const;

  u32 GetCullGroupCount() const noexcept;
  void PopulateDrawCullBarriers(FrameInFlightIndex frame_index,
                                Array<VkBufferMemoryBarrier>& out) const;

  void PrepareShadowCullData(FrameInFlightIndex frame_index,
                             u32 shadow_job_count);
  const ShadowCullBatchRange& GetShadowCullRange(u32 shadow_job_index) const;
  void PopulateShadowCullBarriers(FrameInFlightIndex frame_index,
                                  Array<VkBufferMemoryBarrier>& out) const;

  const Array<RenderBatchGroup>* GetBatchGroups() const noexcept;
  const Array<RenderIndirectBatch>* GetIndirectBatches() const noexcept;

  const Buffer& GetIndirectBuffer(
      FrameInFlightIndex frame_index) const noexcept;
  const Buffer& GetShadowIndirectBuffer(
      FrameInFlightIndex frame_index) const noexcept;
  const Buffer& GetShadowProxyIdsBuffer(
      FrameInFlightIndex frame_index) const noexcept;

  void ReleasePendingUploadResources(FrameInFlightIndex fram_index);

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  static inline constexpr usize kDefaultRenderIndirectBatchCount_{128};
  static inline constexpr usize kDefaultRenderBatchGroupCount_{128};
  static inline constexpr usize kDefaultProxyCount_{512};
  static inline constexpr usize kDefaultShadowCullRangeCount_{16};
  static inline constexpr usize kMaxSkinningMatricesPerModel_{100};
  static inline constexpr usize kDefaultUpdateBarrierCapacity_{4};
  static inline constexpr usize kDefaultCullBarrierCapacity_{2};
  static inline constexpr usize kDefaultSparseUploadBarrierCapacity_{1};

  static inline constexpr f32 kReuploadAllLocalDataThreshold_{.8f};

  static inline constexpr usize kDefaultShaderToTransferBarrierCapacity_{3};

  static bool OnRenderBatchSort(const RenderBatchEntry& a,
                                const RenderBatchEntry& b);

  void GenerateUpdateTemporaryStructures();
  void DestroyUpdateTemporaryStructures();

  void SyncProxyMaterials();
  void RebuildSkinningMatrices(const frame::FramePacket* packet);
  void BuildLocalDataPatches();

  void GenerateBatchEntries();
  void GenerateIndirectBatches();
  void GenerateBatchGroups();

  void EnsureFrameBuffersInitialized(FrameInFlightIndex frame_index);

  void UploadRenderProxyLocalData(FrameInFlightIndex frame_index);
  void UploadAllRenderProxyLocalData(FrameInFlightIndex frame_index);
  void UploadSparseRenderProxyLocalData(FrameInFlightIndex frame_index);

  void UploadRenderProxyIds(FrameInFlightIndex frame_index);
  void UploadMatrixPalettes(FrameInFlightIndex frame_index);

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

  void AddUploadReleaseBarrier(FrameInFlightIndex frame_index,
                               const Buffer& buffer);

#ifdef COMET_DEBUG
  void AssertRuntimeInvariants(FrameInFlightIndex frame_index) const;
  void AssertShadowCullRuntimeInvariants(FrameInFlightIndex frame_index) const;
#endif  // COMET_DEBUG

  void InitializeBuffers();
  void DestroyBuffers();

  void QueueLocalDataPatchForAllFrames(RenderProxyId proxy_id);
  void QueueFrameLocalDataPatches();

  void DestroyLiveProxyMaterials();

  FrameIndex update_frame_{kInvalidFrameIndex};

  u32 render_proxy_count_{0};
  u32 render_proxy_visible_count_{0};

  Array<u32> sparse_patch_word_counts_{};
  usize current_skinning_matrix_count_{0};

  memory::FiberFreeListAllocator batch_allocator_{
      sizeof(RenderBatchEntry) * 32, kDefaultProxyCount_,
      kEngineMemoryTagRender};

  memory::FiberFreeListAllocator matrix_allocator_{
      sizeof(math::Mat4) * 32,
      kDefaultProxyCount_* kMaxSkinningMatricesPerModel_ / 8,
      kEngineMemoryTagRender};

  memory::FiberFreeListAllocator handle_allocator_{
      sizeof(MaterialHandle) * 64, kDefaultProxyCount_,
      kEngineMemoryTagRender};

  memory::PlatformAllocator platform_allocator_{
      kEngineMemoryTagRender};

  Array<MaterialHandle> proxy_material_handles_{};
  Array<RenderBatchEntry> batch_entries_{};
  Array<RenderIndirectBatch> indirect_batches_{};
  Array<RenderBatchGroup> batch_groups_{};

  Array<math::Mat4> skinning_matrices_{};

  Array<ShadowCullBatchRange> shadow_cull_ranges_{};

  Array<Buffer> proxy_local_data_staging_buffers_{};
  Array<Buffer> sparse_upload_source_word_buffers_{};
  Array<Buffer> ssbo_proxy_local_datas_{};

  Array<Buffer> staging_ssbo_proxy_ids_{};
  Array<Buffer> ssbo_proxy_ids_{};

  Array<Buffer> staging_ssbo_matrix_palettes_{};
  Array<Buffer> ssbo_matrix_palettes_{};

  Array<Buffer> ssbo_sparse_upload_word_indices_{};

  Array<Buffer> staging_ssbo_indirect_proxies_{};
  Array<Buffer> ssbo_indirect_proxies_{};

  Array<Buffer> staging_ssbo_shadow_indirect_proxies_{};
  Array<Buffer> ssbo_shadow_indirect_proxies_{};

  Array<Buffer> staging_ssbo_proxy_instances_{};
  Array<Buffer> ssbo_proxy_instances_{};

  Array<Buffer> staging_ssbo_shadow_proxy_ids_{};
  Array<Buffer> ssbo_shadow_proxy_ids_{};

  Array<OrderedSet<RenderProxyId>> pending_local_data_patch_proxy_ids_{};

  const RenderProxyRecordStore* render_proxy_record_store_{nullptr};

  MaterialHandler* material_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  ShaderHandler* shader_handler_{nullptr};

  Array<Array<Buffer>> pending_buffer_destroys_{};

  frame::FrameArray<usize>* local_data_patch_proxy_indices_{nullptr};
  frame::FrameArray<GpuRenderProxyLocalData>* local_data_patch_payloads_{
      nullptr};

  frame::FrameArray<VkBufferMemoryBarrier>* post_update_barriers_{nullptr};
};
}  // namespace vk
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_RENDER_PROXY_HANDLER_H_
