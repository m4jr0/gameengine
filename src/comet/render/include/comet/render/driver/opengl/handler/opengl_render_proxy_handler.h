// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_RENDER_PROXY_HANDLER_H_
#define COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_RENDER_PROXY_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_container.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/core/type/ordered_set.h"
#include "comet/math/matrix.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_material_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_mesh_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_shader_handler.h"
#include "comet/rendering/driver/opengl/type/opengl_buffer.h"
#include "comet/rendering/driver/opengl/type/opengl_frame.h"
#include "comet/rendering/driver/opengl/type/opengl_render_proxy.h"
#include "comet/rendering/driver/opengl/type/opengl_shadow.h"
#include "comet/rendering/render_proxy_record_store.h"
#include "comet/rendering/type/render_proxy.h"

namespace comet {
namespace rendering {
namespace gl {
struct RenderProxySparseUploadData {
  GlNativeStorageHandle ssbo_sparse_upload_word_indices_handle{
      kInvalidGlNativeStorageHandle};
  GLsizei ssbo_sparse_upload_word_indices_size{0};

  GlNativeStorageHandle ssbo_source_words_handle{kInvalidGlNativeStorageHandle};
  GLsizei ssbo_source_words_size{0};

  GlNativeStorageHandle ssbo_destination_words_handle{
      kInvalidGlNativeStorageHandle};
  GLsizei ssbo_destination_words_size{0};

  u32 word_count{0};
};

struct RenderProxyGpuData {
  GlNativeStorageHandle ssbo_proxy_local_datas_handle{
      kInvalidGlNativeStorageHandle};
  GlNativeStorageHandle ssbo_proxy_ids_handle{kInvalidGlNativeStorageHandle};
  GlNativeStorageHandle ssbo_proxy_instances_handle{
      kInvalidGlNativeStorageHandle};
  GlNativeStorageHandle ssbo_indirect_proxies_handle{
      kInvalidGlNativeStorageHandle};
  GlNativeStorageHandle ssbo_matrix_palettes_handle{
      kInvalidGlNativeStorageHandle};

  GLsizei ssbo_proxy_local_datas_size{0};
  GLsizei ssbo_proxy_ids_size{0};
  GLsizei ssbo_proxy_instances_size{0};
  GLsizei ssbo_indirect_proxies_size{0};
  GLsizei ssbo_matrix_palettes_size{0};
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

  u32 GetCullGroupCount() const noexcept;

  void PrepareShadowCullData(FrameInFlightIndex frame_index,
                             u32 shadow_job_count);
  const ShadowCullBatchRange& GetShadowCullRange(u32 shadow_job_index) const;

  const Array<RenderBatchGroup>* GetBatchGroups() const noexcept;
  const Array<RenderIndirectBatch>* GetIndirectBatches() const noexcept;

  GlBufferView GetIndirectBuffer(FrameInFlightIndex frame_index) const noexcept;
  GlBufferView GetShadowIndirectBuffer(
      FrameInFlightIndex frame_index) const noexcept;
  GlBufferView GetShadowProxyIdsBuffer(
      FrameInFlightIndex frame_index) const noexcept;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  static inline constexpr usize kDefaultRenderIndirectBatchCount_{128};
  static inline constexpr usize kDefaultRenderBatchGroupCount_{128};
  static inline constexpr usize kDefaultProxyCount_{512};
  static inline constexpr usize kDefaultShadowCullRangeCount_{16};
  static inline constexpr usize kMaxSkinningMatricesPerModel_{100};

  static inline constexpr f32 kReuploadAllLocalDataThreshold_{.8f};

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

  void PopulateRenderIndirectProxy(BatchId batch_id,
                                   GpuIndirectRenderProxy* memory);
  void PopulateShadowRenderIndirectProxy(BatchId batch_id,
                                         GpuIndirectRenderProxy* memory);
  void PopulateProxyInstances(BatchId batch_id, GpuRenderProxyInstance* memory,
                              usize& proxy_instance_index);

#ifdef COMET_DEBUG
  void AssertRuntimeInvariants(FrameInFlightIndex frame_index) const;
  void AssertShadowCullRuntimeInvariants(FrameInFlightIndex frame_index) const;
#endif  // COMET_DEBUG

  void InitializeBuffers();
  void DestroyBuffers();

  void QueueLocalDataPatchForAllFrames(RenderProxyId proxy_id);
  void QueueFrameLocalDataPatches();

  void DestroyLiveProxyMaterials();

  void EnsureStorageBufferCapacity(GlNativeStorageHandle& handle,
                                   GLsizei& capacity, GLsizei required_size,
                                   GLenum target, const schar* debug_label);
  void UploadStorageBuffer(GlNativeStorageHandle handle, GLsizei size,
                           const void* data, GLenum target);

  FrameCount update_frame_{kInvalidFrameCount};

  u32 render_proxy_count_{0};
  u32 render_proxy_visible_count_{0};

  Array<u32> sparse_patch_word_counts_{};
  usize current_skinning_matrix_count_{0};

  memory::FiberFreeListAllocator batch_allocator_{
      sizeof(RenderBatchEntry) * 32, kDefaultProxyCount_,
      memory::kEngineMemoryTagRendering};

  memory::FiberFreeListAllocator matrix_allocator_{
      sizeof(math::Mat4) * 32,
      kDefaultProxyCount_* kMaxSkinningMatricesPerModel_ / 8,
      memory::kEngineMemoryTagRendering};

  memory::FiberFreeListAllocator handle_allocator_{
      sizeof(MaterialHandle) * 64, kDefaultProxyCount_,
      memory::kEngineMemoryTagRendering};

  memory::PlatformAllocator platform_allocator_{
      memory::kEngineMemoryTagRendering};

  Array<MaterialHandle> proxy_material_handles_{};
  Array<RenderBatchEntry> batch_entries_{};
  Array<RenderIndirectBatch> indirect_batches_{};
  Array<RenderBatchGroup> batch_groups_{};

  Array<math::Mat4> skinning_matrices_{};

  Array<ShadowCullBatchRange> shadow_cull_ranges_{};

  Array<GlNativeStorageHandle> proxy_local_data_staging_buffers_{};
  Array<GlNativeStorageHandle> sparse_upload_source_word_buffers_{};
  Array<GlNativeStorageHandle> ssbo_proxy_local_datas_{};

  Array<GlNativeStorageHandle> staging_ssbo_proxy_ids_{};
  Array<GlNativeStorageHandle> ssbo_proxy_ids_{};

  Array<GlNativeStorageHandle> staging_ssbo_matrix_palettes_{};
  Array<GlNativeStorageHandle> ssbo_matrix_palettes_{};

  Array<GlNativeStorageHandle> ssbo_sparse_upload_word_indices_{};

  Array<GlNativeStorageHandle> staging_ssbo_indirect_proxies_{};
  Array<GlNativeStorageHandle> ssbo_indirect_proxies_{};

  Array<GlNativeStorageHandle> staging_ssbo_shadow_indirect_proxies_{};
  Array<GlNativeStorageHandle> ssbo_shadow_indirect_proxies_{};

  Array<GlNativeStorageHandle> staging_ssbo_proxy_instances_{};
  Array<GlNativeStorageHandle> ssbo_proxy_instances_{};

  Array<GlNativeStorageHandle> staging_ssbo_shadow_proxy_ids_{};
  Array<GlNativeStorageHandle> ssbo_shadow_proxy_ids_{};

  Array<GLsizei> proxy_local_data_staging_buffer_sizes_{};
  Array<GLsizei> sparse_upload_source_word_buffer_sizes_{};
  Array<GLsizei> ssbo_proxy_local_datas_sizes_{};

  Array<GLsizei> staging_ssbo_proxy_ids_sizes_{};
  Array<GLsizei> ssbo_proxy_ids_sizes_{};

  Array<GLsizei> staging_ssbo_matrix_palettes_sizes_{};
  Array<GLsizei> ssbo_matrix_palettes_sizes_{};

  Array<GLsizei> ssbo_sparse_upload_word_indices_sizes_{};

  Array<GLsizei> staging_ssbo_indirect_proxies_sizes_{};
  Array<GLsizei> ssbo_indirect_proxies_sizes_{};

  Array<GLsizei> staging_ssbo_shadow_indirect_proxies_sizes_{};
  Array<GLsizei> ssbo_shadow_indirect_proxies_sizes_{};

  Array<GLsizei> staging_ssbo_proxy_instances_sizes_{};
  Array<GLsizei> ssbo_proxy_instances_sizes_{};

  Array<GLsizei> staging_ssbo_shadow_proxy_ids_sizes_{};
  Array<GLsizei> ssbo_shadow_proxy_ids_sizes_{};

  Array<OrderedSet<RenderProxyId>> pending_local_data_patch_proxy_ids_{};

  const RenderProxyRecordStore* render_proxy_record_store_{nullptr};

  MaterialHandler* material_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  ShaderHandler* shader_handler_{nullptr};

  frame::FrameArray<usize>* local_data_patch_proxy_indices_{nullptr};
  frame::FrameArray<GpuRenderProxyLocalData>* local_data_patch_payloads_{
      nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_RENDER_PROXY_HANDLER_H_