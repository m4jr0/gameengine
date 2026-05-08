// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_render_proxy_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/algorithm/sort.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/ordered_set.h"
#include "comet/math/matrix.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/opengl_debug.h"
#include "comet/rendering/driver/opengl/type/opengl_mesh.h"
#include "comet/rendering/type/shader.h"

namespace comet {
namespace rendering {
namespace gl {
RenderProxyHandler::RenderProxyHandler(const RenderProxyHandlerDescr& descr)
    : Handler{descr},
      render_proxy_record_store_{descr.render_proxy_record_store},
      material_handler_{descr.material_handler},
      mesh_handler_{descr.mesh_handler},
      shader_handler_{descr.shader_handler} {
  COMET_ASSERT(render_proxy_record_store_ != nullptr,
               "RenderProxyHandler::RenderProxyHandler",
               "render proxy record store is null");
  COMET_ASSERT(material_handler_ != nullptr,
               "RenderProxyHandler::RenderProxyHandler",
               "material handler is null");
  COMET_ASSERT(mesh_handler_ != nullptr,
               "RenderProxyHandler::RenderProxyHandler",
               "mesh handler is null");
  COMET_ASSERT(shader_handler_ != nullptr,
               "RenderProxyHandler::RenderProxyHandler",
               "shader handler is null");
}

void RenderProxyHandler::Update(frame::FramePacket* packet) {
  COMET_PROFILE("RenderProxyHandler::Update");

  COMET_ASSERT(packet != nullptr, "RenderProxyHandler::Update",
               "frame packet is null");

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};

  GenerateUpdateTemporaryStructures();
  EnsureFrameBuffersInitialized(frame_index);

  SyncProxyMaterials();
  RebuildSkinningMatrices(packet);
  QueueFrameLocalDataPatches();
  BuildLocalDataPatches();

  render_proxy_count_ =
      static_cast<u32>(render_proxy_record_store_->GetRenderProxyCount());
  current_skinning_matrix_count_ = skinning_matrices_.GetSize();

  GenerateBatchEntries();
  GenerateIndirectBatches();
  GenerateBatchGroups();

  UploadRenderProxyLocalData(frame_index);
  UploadRenderProxyIds(frame_index);
  UploadMatrixPalettes(frame_index);
  PrepareRenderProxyDrawData(frame_index);

#ifdef COMET_DEBUG
  AssertRuntimeInvariants(frame_index);
#endif  // COMET_DEBUG

  update_frame_ = static_cast<FrameCount>(packet->frame_count);
}

void RenderProxyHandler::Reset() {
  DestroyUpdateTemporaryStructures();

  for (usize i{0}; i < sparse_patch_word_counts_.GetSize(); ++i) {
    sparse_patch_word_counts_[i] = 0;
  }
}

u32 RenderProxyHandler::GetRenderProxyCount() const noexcept {
  return static_cast<u32>(render_proxy_count_);
}

u32 RenderProxyHandler::GetVisibleCount() const noexcept {
  return static_cast<u32>(render_proxy_visible_count_);
}

u32 RenderProxyHandler::GetRenderProxyInstanceCount() const noexcept {
  return static_cast<u32>(batch_entries_.GetSize());
}

MaterialHandle RenderProxyHandler::GetMaterialHandle(
    RenderProxyId proxy_id) const {
  COMET_ASSERT(proxy_id < proxy_material_handles_.GetSize(),
               "RenderProxyHandler::GetMaterialHandle", "proxy id out of range",
               "proxy_id", proxy_id, "material_handle_count",
               proxy_material_handles_.GetSize());

  const auto material_handle{proxy_material_handles_[proxy_id]};

  COMET_ASSERT(material_handle, "RenderProxyHandler::GetMaterialHandle",
               "material handle is invalid", "proxy_id", proxy_id);

  return material_handle;
}

RenderProxyGpuData RenderProxyHandler::GetGpuData(
    FrameInFlightIndex frame_index) const noexcept {
  RenderProxyGpuData gpu_data{};

  gpu_data.ssbo_proxy_local_datas_handle = ssbo_proxy_local_datas_[frame_index];
  gpu_data.ssbo_proxy_local_datas_size =
      ssbo_proxy_local_datas_sizes_[frame_index];

  gpu_data.ssbo_proxy_ids_handle = ssbo_proxy_ids_[frame_index];
  gpu_data.ssbo_proxy_ids_size = ssbo_proxy_ids_sizes_[frame_index];

  gpu_data.ssbo_matrix_palettes_handle = ssbo_matrix_palettes_[frame_index];
  gpu_data.ssbo_matrix_palettes_size = ssbo_matrix_palettes_sizes_[frame_index];

  gpu_data.ssbo_proxy_instances_handle = ssbo_proxy_instances_[frame_index];
  gpu_data.ssbo_proxy_instances_size = ssbo_proxy_instances_sizes_[frame_index];

  gpu_data.ssbo_indirect_proxies_handle = ssbo_indirect_proxies_[frame_index];
  gpu_data.ssbo_indirect_proxies_size =
      ssbo_indirect_proxies_sizes_[frame_index];

  return gpu_data;
}

bool RenderProxyHandler::HasPendingSparseUpload(
    FrameInFlightIndex frame_index) const noexcept {
  if (frame_index >= sparse_patch_word_counts_.GetSize()) {
    return false;
  }

  return sparse_patch_word_counts_[frame_index] > 0;
}

u32 RenderProxyHandler::GetSparseUploadWordCount(
    FrameInFlightIndex frame_index) const noexcept {
  if (frame_index >= sparse_patch_word_counts_.GetSize()) {
    return 0;
  }

  return sparse_patch_word_counts_[frame_index];
}

u32 RenderProxyHandler::GetSparseUploadGroupCount(
    FrameInFlightIndex frame_index) const noexcept {
  const auto word_count{GetSparseUploadWordCount(frame_index)};
  return static_cast<u32>((word_count + kShaderLocalSize - 1) /
                          kShaderLocalSize);
}

RenderProxySparseUploadData RenderProxyHandler::GetSparseUploadGpuData(
    FrameInFlightIndex frame_index) const noexcept {
  RenderProxySparseUploadData gpu_data{};

  if (frame_index >= ssbo_sparse_upload_word_indices_.GetSize() ||
      frame_index >= sparse_upload_source_word_buffers_.GetSize() ||
      frame_index >= ssbo_proxy_local_datas_.GetSize()) {
    return gpu_data;
  }

  gpu_data.ssbo_sparse_upload_word_indices_handle =
      ssbo_sparse_upload_word_indices_[frame_index];
  gpu_data.ssbo_sparse_upload_word_indices_size =
      ssbo_sparse_upload_word_indices_sizes_[frame_index];

  gpu_data.ssbo_source_words_handle =
      sparse_upload_source_word_buffers_[frame_index];
  gpu_data.ssbo_source_words_size =
      sparse_upload_source_word_buffer_sizes_[frame_index];

  gpu_data.ssbo_destination_words_handle = ssbo_proxy_local_datas_[frame_index];
  gpu_data.ssbo_destination_words_size =
      ssbo_proxy_local_datas_sizes_[frame_index];

  gpu_data.word_count = sparse_patch_word_counts_[frame_index];
  return gpu_data;
}

u32 RenderProxyHandler::GetCullGroupCount() const noexcept {
  return static_cast<u32>((batch_entries_.GetSize() + kShaderLocalSize - 1) /
                          kShaderLocalSize);
}

void RenderProxyHandler::PrepareShadowCullData(FrameInFlightIndex frame_index,
                                               u32 shadow_job_count) {
  COMET_PROFILE("RenderProxyHandler::PrepareShadowCullData");

  shadow_cull_ranges_.Clear();

  if (shadow_job_count == 0 || indirect_batches_.IsEmpty()) {
    return;
  }

  const auto batch_count{static_cast<u32>(indirect_batches_.GetSize())};
  const auto instance_count{static_cast<u32>(batch_entries_.GetSize())};

  shadow_cull_ranges_.Reserve(shadow_job_count);

  for (u32 job_index{0}; job_index < shadow_job_count; ++job_index) {
    auto& range{shadow_cull_ranges_.EmplaceLast()};
    range.indirect_offset = job_index * batch_count;
    range.proxy_id_offset = job_index * instance_count;
    range.batch_count = batch_count;
    range.instance_capacity = instance_count;
  }

  const auto indirect_size{static_cast<GLsizei>(
      shadow_job_count * batch_count * sizeof(GpuIndirectRenderProxy))};
  const auto proxy_ids_size{static_cast<GLsizei>(
      shadow_job_count * instance_count * sizeof(RenderProxyId))};

  EnsureStorageBufferCapacity(
      staging_ssbo_shadow_indirect_proxies_[frame_index],
      staging_ssbo_shadow_indirect_proxies_sizes_[frame_index], indirect_size,
      GL_SHADER_STORAGE_BUFFER, "staging_ssbo_shadow_indirect_proxies_");

  EnsureStorageBufferCapacity(ssbo_shadow_indirect_proxies_[frame_index],
                              ssbo_shadow_indirect_proxies_sizes_[frame_index],
                              indirect_size, GL_DRAW_INDIRECT_BUFFER,
                              "ssbo_shadow_indirect_proxies_");

  EnsureStorageBufferCapacity(ssbo_shadow_proxy_ids_[frame_index],
                              ssbo_shadow_proxy_ids_sizes_[frame_index],
                              proxy_ids_size, GL_SHADER_STORAGE_BUFFER,
                              "ssbo_shadow_proxy_ids_");

  glBindBuffer(GL_SHADER_STORAGE_BUFFER,
               staging_ssbo_shadow_indirect_proxies_[frame_index]);
  auto* memory{static_cast<GpuIndirectRenderProxy*>(
      glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY))};

  COMET_ASSERT(memory != nullptr, "RenderProxyHandler::PrepareShadowCullData",
               "failed to map shadow indirect staging buffer");

  for (u32 job_index{0}; job_index < shadow_job_count; ++job_index) {
    const auto& range{shadow_cull_ranges_[job_index]};

    for (u32 batch_index{0}; batch_index < batch_count; ++batch_index) {
      const auto dst_index{range.indirect_offset + batch_index};

      PopulateShadowRenderIndirectProxy(static_cast<BatchId>(batch_index),
                                        memory + range.indirect_offset);

      auto& proxy{memory[dst_index]};
      proxy.command.instanceCount = 0;
      proxy.command.firstInstance =
          range.proxy_id_offset + indirect_batches_[batch_index].offset;
      proxy.batch_id = batch_index;
    }
  }

  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  glBindBuffer(GL_COPY_READ_BUFFER,
               staging_ssbo_shadow_indirect_proxies_[frame_index]);
  glBindBuffer(GL_COPY_WRITE_BUFFER,
               ssbo_shadow_indirect_proxies_[frame_index]);
  glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
                      indirect_size);
  glBindBuffer(GL_COPY_READ_BUFFER, kInvalidGlNativeStorageHandle);
  glBindBuffer(GL_COPY_WRITE_BUFFER, kInvalidGlNativeStorageHandle);

#ifdef COMET_DEBUG
  AssertShadowCullRuntimeInvariants(frame_index);
#endif  // COMET_DEBUG
}

const ShadowCullBatchRange& RenderProxyHandler::GetShadowCullRange(
    u32 shadow_job_index) const {
  COMET_ASSERT(shadow_job_index < shadow_cull_ranges_.GetSize(),
               "RenderProxyHandler::GetShadowCullRange",
               "shadow job index out of bounds", "shadow_job_index",
               shadow_job_index, "range_count", shadow_cull_ranges_.GetSize());

  return shadow_cull_ranges_[shadow_job_index];
}

const Array<RenderBatchGroup>* RenderProxyHandler::GetBatchGroups()
    const noexcept {
  return &batch_groups_;
}

const Array<RenderIndirectBatch>* RenderProxyHandler::GetIndirectBatches()
    const noexcept {
  return &indirect_batches_;
}

GlBufferView RenderProxyHandler::GetIndirectBuffer(
    FrameInFlightIndex frame_index) const noexcept {
  return {
      ssbo_indirect_proxies_[frame_index],
      ssbo_indirect_proxies_sizes_[frame_index],
  };
}

GlBufferView RenderProxyHandler::GetShadowIndirectBuffer(
    FrameInFlightIndex frame_index) const noexcept {
  return {
      ssbo_shadow_indirect_proxies_[frame_index],
      ssbo_shadow_indirect_proxies_sizes_[frame_index],
  };
}

GlBufferView RenderProxyHandler::GetShadowProxyIdsBuffer(
    FrameInFlightIndex frame_index) const noexcept {
  return {
      ssbo_shadow_proxy_ids_[frame_index],
      ssbo_shadow_proxy_ids_sizes_[frame_index],
  };
}

void RenderProxyHandler::OnInitialize() {
  batch_allocator_.Initialize();
  matrix_allocator_.Initialize();
  handle_allocator_.Initialize();

  proxy_material_handles_ = Array<MaterialHandle>::WithCapacity(
      &handle_allocator_, kDefaultProxyCount_);
  batch_entries_ = Array<RenderBatchEntry>::WithCapacity(&batch_allocator_,
                                                         kDefaultProxyCount_);
  indirect_batches_ = Array<RenderIndirectBatch>::WithCapacity(
      &batch_allocator_, kDefaultRenderIndirectBatchCount_);
  batch_groups_ = Array<RenderBatchGroup>::WithCapacity(
      &batch_allocator_, kDefaultRenderBatchGroupCount_);

  skinning_matrices_ = Array<math::Mat4>::WithCapacity(
      &matrix_allocator_, kDefaultProxyCount_ * kMaxSkinningMatricesPerModel_);

  shadow_cull_ranges_ = Array<ShadowCullBatchRange>::WithCapacity(
      &platform_allocator_, kDefaultShadowCullRangeCount_);

  const auto max_frames_in_flight{frame_state_->GetMaxFramesInFlight()};

  sparse_patch_word_counts_ =
      Array<u32>::WithCapacity(&platform_allocator_, max_frames_in_flight);
  sparse_patch_word_counts_.Resize(max_frames_in_flight);

  for (usize i{0}; i < sparse_patch_word_counts_.GetSize(); ++i) {
    sparse_patch_word_counts_[i] = 0;
  }

  render_proxy_count_ = 0;
  render_proxy_visible_count_ = 0;
  current_skinning_matrix_count_ = 0;
  update_frame_ = kInvalidFrameCount;

  InitializeBuffers();
}

void RenderProxyHandler::OnShutdown() {
  DestroyLiveProxyMaterials();
  DestroyUpdateTemporaryStructures();
  DestroyBuffers();

  shadow_cull_ranges_.Release();
  skinning_matrices_.Release();
  batch_groups_.Release();
  indirect_batches_.Release();
  batch_entries_.Release();
  proxy_material_handles_.Release();

  batch_allocator_.Destroy();
  matrix_allocator_.Destroy();
  handle_allocator_.Destroy();

  update_frame_ = kInvalidFrameCount;
  render_proxy_count_ = 0;
  render_proxy_visible_count_ = 0;
  current_skinning_matrix_count_ = 0;
  sparse_patch_word_counts_.Release();
}

bool RenderProxyHandler::OnRenderBatchSort(const RenderBatchEntry& a,
                                           const RenderBatchEntry& b) {
  if (a.sort_key != b.sort_key) {
    return a.sort_key < b.sort_key;
  }

  return a.proxy_id < b.proxy_id;
}

void RenderProxyHandler::GenerateUpdateTemporaryStructures() {
  const auto patch_capacity{
      render_proxy_record_store_->GetDirtyProxyIds().GetSize() +
      render_proxy_record_store_->GetRemovedProxyIds().GetSize() +
      kDefaultProxyCount_};

  local_data_patch_payloads_ =
      COMET_FRAME_ARRAY_WITH_CAPACITY(GpuRenderProxyLocalData, patch_capacity);

  local_data_patch_proxy_indices_ =
      COMET_FRAME_ARRAY_WITH_CAPACITY(usize, patch_capacity);
}

void RenderProxyHandler::DestroyUpdateTemporaryStructures() {
  local_data_patch_payloads_ = nullptr;
  local_data_patch_proxy_indices_ = nullptr;
}

void RenderProxyHandler::SyncProxyMaterials() {
  COMET_PROFILE("RenderProxyHandler::SyncProxyMaterials");

  const auto required_size{render_proxy_record_store_->GetRecordSlotCount()};

  if (proxy_material_handles_.GetSize() < required_size) {
    proxy_material_handles_.Resize(required_size);
  }

  for (const auto proxy_id : render_proxy_record_store_->GetRemovedProxyIds()) {
    if (proxy_id >= proxy_material_handles_.GetSize()) {
      continue;
    }

    auto& material_handle{proxy_material_handles_[proxy_id]};

    if (material_handle) {
      material_handler_->Destroy(material_handle);
      material_handle.Invalidate();
    }
  }

  for (const auto proxy_id : render_proxy_record_store_->GetAddedProxyIds()) {
    const auto& record{render_proxy_record_store_->GetRecord(proxy_id)};

    if (proxy_id >= proxy_material_handles_.GetSize()) {
      proxy_material_handles_.Resize(proxy_id + 1);
    }

    auto& material_handle{proxy_material_handles_[proxy_id]};

    if (material_handle) {
      material_handler_->Destroy(material_handle);
      material_handle.Invalidate();
    }

    COMET_ASSERT(record.material_resource_id,
                 "RenderProxyHandler::SyncProxyMaterials",
                 "material resource id is invalid", "proxy_id", proxy_id,
                 "proxy_entity_id", record.proxy.entity_id);

    material_handle =
        material_handler_->GetOrGenerate(record.material_resource_id);
    COMET_ASSERT(material_handle, "RenderProxyHandler::SyncProxyMaterials",
                 "material handle is invalid", "proxy_id", proxy_id,
                 "proxy_entity_id", record.proxy.entity_id, "proxy_mesh_handle",
                 record.proxy.mesh_handle, "material_resource_id",
                 record.material_resource_id);
  }
}

void RenderProxyHandler::RebuildSkinningMatrices(
    const frame::FramePacket* packet) {
  COMET_PROFILE("RenderProxyHandler::RebuildSkinningMatrices");

  COMET_ASSERT(packet != nullptr, "RenderProxyHandler::RebuildSkinningMatrices",
               "frame packet is null");

  skinning_matrices_.Clear();

  usize total_joint_count{0};

  for (const auto& palette : *packet->matrix_palettes) {
    total_joint_count += palette.skinning_matrix_count;
  }

  if (total_joint_count == 0) {
    return;
  }

  if (skinning_matrices_.GetCapacity() < total_joint_count) {
    skinning_matrices_.Reserve(total_joint_count);
  }

  for (const auto& palette : *packet->matrix_palettes) {
    for (usize i{0}; i < palette.skinning_matrix_count; ++i) {
      skinning_matrices_.PushLast(palette.skinning_matrices[i]);
    }
  }
}

void RenderProxyHandler::BuildLocalDataPatches() {
  COMET_PROFILE("RenderProxyHandler::BuildLocalDataPatches");

  COMET_ASSERT(local_data_patch_proxy_indices_ != nullptr,
               "RenderProxyHandler::BuildLocalDataPatches",
               "local data patch proxy indices are null");
  COMET_ASSERT(local_data_patch_payloads_ != nullptr,
               "RenderProxyHandler::BuildLocalDataPatches",
               "local data patch payloads are null");

  local_data_patch_proxy_indices_->Clear();
  local_data_patch_payloads_->Clear();

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};

  COMET_ASSERT(frame_index < pending_local_data_patch_proxy_ids_.GetSize(),
               "RenderProxyHandler::BuildLocalDataPatches",
               "frame index out of bounds", "frame_index", frame_index,
               "pending_frame_count",
               pending_local_data_patch_proxy_ids_.GetSize());

  auto& pending_proxy_ids{pending_local_data_patch_proxy_ids_[frame_index]};

  for (const auto proxy_id : pending_proxy_ids) {
    local_data_patch_proxy_indices_->PushLast(static_cast<usize>(proxy_id));
    const auto* record{render_proxy_record_store_->TryGetRecord(proxy_id)};

    if (record != nullptr) {
      local_data_patch_payloads_->PushLast(record->local_data);
    } else {
      local_data_patch_payloads_->PushLast(GpuRenderProxyLocalData{});
    }
  }

  pending_proxy_ids.Clear();
}

void RenderProxyHandler::GenerateBatchEntries() {
  COMET_PROFILE("RenderProxyHandler::GenerateBatchEntries");

  batch_entries_.Clear();

  const auto& active_proxy_ids{render_proxy_record_store_->GetActiveProxyIds()};

  if (active_proxy_ids.IsEmpty()) {
    return;
  }

  if (batch_entries_.GetCapacity() < active_proxy_ids.GetSize()) {
    batch_entries_.Reserve(active_proxy_ids.GetSize());
  }

  for (const auto proxy_id : active_proxy_ids) {
    const auto& record{render_proxy_record_store_->GetRecord(proxy_id)};

    if (mesh_handler_->TryGet(record.proxy.mesh_handle) == nullptr) {
      continue;
    }

    COMET_ASSERT(proxy_id < proxy_material_handles_.GetSize(),
                 "RenderProxyHandler::GenerateBatchEntries",
                 "material handle index out of range", "proxy_id", proxy_id,
                 "material_handle_count", proxy_material_handles_.GetSize());

    const auto material_handle{proxy_material_handles_[proxy_id]};

    COMET_ASSERT(material_handle, "RenderProxyHandler::GenerateBatchEntries",
                 "material handle is invalid", "proxy_id", proxy_id,
                 "entity_id", record.proxy.entity_id);

    auto& batch{batch_entries_.EmplaceLast()};
    batch.proxy_id = proxy_id;
    batch.sort_key = (static_cast<u64>(GenerateHash(material_handle)) << 32) |
                     HashCombine(GenerateHash(material_handle),
                                 GenerateHash(record.proxy.mesh_handle));
  }

  Sort(batch_entries_.begin(), batch_entries_.end(), OnRenderBatchSort);
}

void RenderProxyHandler::GenerateIndirectBatches() {
  COMET_PROFILE("RenderProxyHandler::GenerateIndirectBatches");

  indirect_batches_.Clear();

  if (batch_entries_.IsEmpty()) {
    return;
  }

  const auto first_proxy_id{batch_entries_[0].proxy_id};
  auto& first_batch{indirect_batches_.EmplaceLast()};
  first_batch.offset = 0;
  first_batch.count = 1;
  first_batch.proxy_id = first_proxy_id;

  usize last_batch_index{0};

  for (usize batch_id{1}; batch_id < batch_entries_.GetSize(); ++batch_id) {
    const auto proxy_id{batch_entries_[batch_id].proxy_id};
    const auto& record{render_proxy_record_store_->GetRecord(proxy_id)};

    auto& last_batch{indirect_batches_[last_batch_index]};
    const auto& last_record{
        render_proxy_record_store_->GetRecord(last_batch.proxy_id)};

    COMET_ASSERT(proxy_id < proxy_material_handles_.GetSize(),
                 "RenderProxyHandler::GenerateIndirectBatches",
                 "proxy id out of range", "proxy_id", proxy_id,
                 "material_handle_count", proxy_material_handles_.GetSize());

    COMET_ASSERT(last_batch.proxy_id < proxy_material_handles_.GetSize(),
                 "RenderProxyHandler::GenerateIndirectBatches",
                 "last proxy id out of range", "proxy_id", last_batch.proxy_id,
                 "material_handle_count", proxy_material_handles_.GetSize());

    const auto material_handle{proxy_material_handles_[proxy_id]};
    const auto last_material_handle{
        proxy_material_handles_[last_batch.proxy_id]};

    const auto is_same_mesh{record.proxy.mesh_handle ==
                            last_record.proxy.mesh_handle};
    const auto is_same_material{material_handle == last_material_handle};

    if (is_same_mesh && is_same_material) {
      ++last_batch.count;
      continue;
    }

    auto& new_batch{indirect_batches_.EmplaceLast()};
    new_batch.offset = static_cast<u32>(batch_id);
    new_batch.count = 1;
    new_batch.proxy_id = proxy_id;
    last_batch_index = indirect_batches_.GetSize() - 1;
  }
}

void RenderProxyHandler::GenerateBatchGroups() {
  COMET_PROFILE("RenderProxyHandler::GenerateBatchGroups");

  batch_groups_.Clear();

  if (indirect_batches_.IsEmpty()) {
    return;
  }

  auto& first_group{batch_groups_.EmplaceLast()};
  first_group.offset = 0;
  first_group.count = 1;

  for (usize i{1}; i < indirect_batches_.GetSize(); ++i) {
    auto& current_group{batch_groups_.GetLast()};

    const auto anchor_proxy_id{
        indirect_batches_[current_group.offset].proxy_id};
    const auto proxy_id{indirect_batches_[i].proxy_id};

    COMET_ASSERT(anchor_proxy_id < proxy_material_handles_.GetSize(),
                 "RenderProxyHandler::GenerateBatchGroups",
                 "anchor proxy id out of range", "anchor_proxy_id",
                 anchor_proxy_id, "material_handle_count",
                 proxy_material_handles_.GetSize());
    COMET_ASSERT(proxy_id < proxy_material_handles_.GetSize(),
                 "RenderProxyHandler::GenerateBatchGroups",
                 "proxy id out of range", "proxy_id", proxy_id,
                 "material_handle_count", proxy_material_handles_.GetSize());

    if (proxy_material_handles_[anchor_proxy_id] ==
        proxy_material_handles_[proxy_id]) {
      ++current_group.count;
      continue;
    }

    auto& new_group{batch_groups_.EmplaceLast()};
    new_group.offset = static_cast<u32>(i);
    new_group.count = 1;
  }
}

void RenderProxyHandler::EnsureFrameBuffersInitialized(
    FrameInFlightIndex frame_index) {
  COMET_ASSERT(frame_index < frame_state_->GetMaxFramesInFlight(),
               "RenderProxyHandler::EnsureFrameBuffersInitialized",
               "frame index out of bounds", "frame_index", frame_index,
               "max_frames_in_flight", frame_state_->GetMaxFramesInFlight());

  const auto ensure_buffer{[](GlNativeStorageHandle& handle,
                              [[maybe_unused]] const schar* debug_label) {
    if (handle != kInvalidGlNativeStorageHandle) {
      return;
    }

    glCreateBuffers(1, &handle);
    COMET_ASSERT(handle != kInvalidGlNativeStorageHandle,
                 "RenderProxyHandler::EnsureFrameBuffersInitialized",
                 "failed to create buffer", "debug_label", debug_label);
    COMET_GL_SET_STORAGE_DEBUG_LABEL(handle, debug_label);
  }};

  ensure_buffer(proxy_local_data_staging_buffers_[frame_index],
                "proxy_local_data_staging_buffers_");
  ensure_buffer(sparse_upload_source_word_buffers_[frame_index],
                "sparse_upload_source_word_buffers_");
  ensure_buffer(ssbo_proxy_local_datas_[frame_index],
                "ssbo_proxy_local_datas_");

  ensure_buffer(staging_ssbo_proxy_ids_[frame_index],
                "staging_ssbo_proxy_ids_");
  ensure_buffer(ssbo_proxy_ids_[frame_index], "ssbo_proxy_ids_");

  ensure_buffer(staging_ssbo_matrix_palettes_[frame_index],
                "staging_ssbo_matrix_palettes_");
  ensure_buffer(ssbo_matrix_palettes_[frame_index], "ssbo_matrix_palettes_");

  ensure_buffer(ssbo_sparse_upload_word_indices_[frame_index],
                "ssbo_sparse_upload_word_indices_");

  ensure_buffer(staging_ssbo_indirect_proxies_[frame_index],
                "staging_ssbo_indirect_proxies_");
  ensure_buffer(ssbo_indirect_proxies_[frame_index], "ssbo_indirect_proxies_");

  ensure_buffer(staging_ssbo_shadow_indirect_proxies_[frame_index],
                "staging_ssbo_shadow_indirect_proxies_");
  ensure_buffer(ssbo_shadow_indirect_proxies_[frame_index],
                "ssbo_shadow_indirect_proxies_");

  ensure_buffer(staging_ssbo_proxy_instances_[frame_index],
                "staging_ssbo_proxy_instances_");
  ensure_buffer(ssbo_proxy_instances_[frame_index], "ssbo_proxy_instances_");

  ensure_buffer(staging_ssbo_shadow_proxy_ids_[frame_index],
                "staging_ssbo_shadow_proxy_ids_");
  ensure_buffer(ssbo_shadow_proxy_ids_[frame_index], "ssbo_shadow_proxy_ids_");
}

void RenderProxyHandler::UploadSparseRenderProxyLocalData(
    FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::UploadSparseRenderProxyLocalData");

  COMET_ASSERT(local_data_patch_proxy_indices_ != nullptr,
               "RenderProxyHandler::UploadSparseRenderProxyLocalData",
               "local data patch proxy indices are null");
  COMET_ASSERT(local_data_patch_payloads_ != nullptr,
               "RenderProxyHandler::UploadSparseRenderProxyLocalData",
               "local data patch payloads are null");

  const auto pending_count{local_data_patch_payloads_->GetSize()};

  if (pending_count == 0) {
    sparse_patch_word_counts_[frame_index] = 0;
    return;
  }

  COMET_ASSERT(local_data_patch_proxy_indices_->GetSize() == pending_count,
               "RenderProxyHandler::UploadSparseRenderProxyLocalData",
               "local data patch index/payload count mismatch", "index_count",
               local_data_patch_proxy_indices_->GetSize(), "payload_count",
               pending_count);

  static_assert(sizeof(GpuRenderProxyLocalData) % sizeof(u32) == 0,
                "GpuRenderProxyLocalData size must be word aligned");

  constexpr auto kWordSize{sizeof(u32)};
  constexpr auto kLocalDataWordCount{sizeof(GpuRenderProxyLocalData) /
                                     kWordSize};

  const auto word_count{static_cast<u32>(pending_count * kLocalDataWordCount)};

  const auto record_slot_count{
      render_proxy_record_store_->GetRecordSlotCount()};
  const auto destination_size{static_cast<GLsizei>(
      record_slot_count * sizeof(GpuRenderProxyLocalData))};

  EnsureStorageBufferCapacity(ssbo_proxy_local_datas_[frame_index],
                              ssbo_proxy_local_datas_sizes_[frame_index],
                              destination_size, GL_SHADER_STORAGE_BUFFER,
                              "ssbo_proxy_local_datas_");

  const auto word_indices_size{static_cast<GLsizei>(word_count * sizeof(u32))};
  const auto source_words_size{
      static_cast<GLsizei>(pending_count * sizeof(GpuRenderProxyLocalData))};

  EnsureStorageBufferCapacity(
      ssbo_sparse_upload_word_indices_[frame_index],
      ssbo_sparse_upload_word_indices_sizes_[frame_index], word_indices_size,
      GL_SHADER_STORAGE_BUFFER, "ssbo_sparse_upload_word_indices_");

  EnsureStorageBufferCapacity(
      sparse_upload_source_word_buffers_[frame_index],
      sparse_upload_source_word_buffer_sizes_[frame_index], source_words_size,
      GL_SHADER_STORAGE_BUFFER, "sparse_upload_source_word_buffers_");

  glBindBuffer(GL_SHADER_STORAGE_BUFFER,
               ssbo_sparse_upload_word_indices_[frame_index]);
  auto* word_indices{
      static_cast<u32*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY))};
  COMET_ASSERT(word_indices != nullptr,
               "RenderProxyHandler::UploadSparseRenderProxyLocalData",
               "failed to map sparse word indices buffer");

  glBindBuffer(GL_COPY_WRITE_BUFFER,
               sparse_upload_source_word_buffers_[frame_index]);
  auto* source_memory{static_cast<GpuRenderProxyLocalData*>(
      glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY))};
  COMET_ASSERT(source_memory != nullptr,
               "RenderProxyHandler::UploadSparseRenderProxyLocalData",
               "failed to map sparse source words buffer");

  for (usize i{0}; i < pending_count; ++i) {
    const auto proxy_index{(*local_data_patch_proxy_indices_)[i]};

    COMET_ASSERT(proxy_index < record_slot_count,
                 "RenderProxyHandler::UploadSparseRenderProxyLocalData",
                 "proxy patch index out of bounds", "proxy_index", proxy_index,
                 "record_slot_count", record_slot_count);

    source_memory[i] = (*local_data_patch_payloads_)[i];

    const auto destination_word_base{
        static_cast<u32>(proxy_index * kLocalDataWordCount)};
    const auto source_word_base{static_cast<u32>(i * kLocalDataWordCount)};

    for (u32 word_offset{0}; word_offset < kLocalDataWordCount; ++word_offset) {
      const auto word_index{static_cast<usize>(source_word_base + word_offset)};
      word_indices[word_index] = destination_word_base + word_offset;
    }
  }

  glBindBuffer(GL_SHADER_STORAGE_BUFFER,
               ssbo_sparse_upload_word_indices_[frame_index]);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  glBindBuffer(GL_COPY_WRITE_BUFFER,
               sparse_upload_source_word_buffers_[frame_index]);
  glUnmapBuffer(GL_COPY_WRITE_BUFFER);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
  glBindBuffer(GL_COPY_WRITE_BUFFER, kInvalidGlNativeStorageHandle);

  sparse_patch_word_counts_[frame_index] = word_count;
}

void RenderProxyHandler::UploadRenderProxyLocalData(
    FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::UploadRenderProxyLocalData");

  const auto sparse_upload_count{local_data_patch_payloads_ == nullptr
                                     ? 0
                                     : local_data_patch_payloads_->GetSize()};

  if (sparse_upload_count == 0) {
    sparse_patch_word_counts_[frame_index] = 0;
    return;
  }

  const auto record_slot_count{
      render_proxy_record_store_->GetRecordSlotCount()};
  const auto destination_size{static_cast<GLsizei>(
      record_slot_count * sizeof(GpuRenderProxyLocalData))};

  const bool needs_grow{
      ssbo_proxy_local_datas_[frame_index] == kInvalidGlNativeStorageHandle ||
      ssbo_proxy_local_datas_sizes_[frame_index] < destination_size};

  if (needs_grow || static_cast<f32>(sparse_upload_count) >
                        static_cast<f32>(record_slot_count) *
                            kReuploadAllLocalDataThreshold_) {
    UploadAllRenderProxyLocalData(frame_index);
  } else {
    UploadSparseRenderProxyLocalData(frame_index);
  }
}

void RenderProxyHandler::UploadAllRenderProxyLocalData(
    FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::UploadAllRenderProxyLocalData");

  const auto record_slot_count{
      render_proxy_record_store_->GetRecordSlotCount()};

  if (record_slot_count == 0) {
    sparse_patch_word_counts_[frame_index] = 0;
    return;
  }

  const auto size{static_cast<GLsizei>(record_slot_count *
                                       sizeof(GpuRenderProxyLocalData))};

  EnsureStorageBufferCapacity(ssbo_proxy_local_datas_[frame_index],
                              ssbo_proxy_local_datas_sizes_[frame_index], size,
                              GL_SHADER_STORAGE_BUFFER,
                              "ssbo_proxy_local_datas_");

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_proxy_local_datas_[frame_index]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

  auto* memory{static_cast<GpuRenderProxyLocalData*>(
      glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY))};
  COMET_ASSERT(memory != nullptr,
               "RenderProxyHandler::UploadAllRenderProxyLocalData",
               "failed to map proxy local data buffer");

  memory::Memset(memory, 0, static_cast<usize>(size));

  const auto& active_proxy_ids{render_proxy_record_store_->GetActiveProxyIds()};

  for (const auto proxy_id : active_proxy_ids) {
    COMET_ASSERT(proxy_id < record_slot_count,
                 "RenderProxyHandler::UploadAllRenderProxyLocalData",
                 "proxy id out of range", "proxy_id", proxy_id,
                 "record_slot_count", record_slot_count);
    memory[proxy_id] =
        render_proxy_record_store_->GetRecord(proxy_id).local_data;
  }

  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);

  ssbo_proxy_local_datas_sizes_[frame_index] = size;
  sparse_patch_word_counts_[frame_index] = 0;
}

void RenderProxyHandler::UploadRenderProxyIds(FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::UploadRenderProxyIds");

  const auto& active_proxy_ids{render_proxy_record_store_->GetActiveProxyIds()};

  if (active_proxy_ids.IsEmpty()) {
    return;
  }

  const auto size{
      static_cast<GLsizei>(active_proxy_ids.GetSize() * sizeof(RenderProxyId))};

  EnsureStorageBufferCapacity(staging_ssbo_proxy_ids_[frame_index],
                              staging_ssbo_proxy_ids_sizes_[frame_index], size,
                              GL_SHADER_STORAGE_BUFFER,
                              "staging_ssbo_proxy_ids_");

  EnsureStorageBufferCapacity(ssbo_proxy_ids_[frame_index],
                              ssbo_proxy_ids_sizes_[frame_index], size,
                              GL_SHADER_STORAGE_BUFFER, "ssbo_proxy_ids_");

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_proxy_ids_[frame_index]);
  auto* memory{static_cast<RenderProxyId*>(
      glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY))};

  COMET_ASSERT(memory != nullptr, "RenderProxyHandler::UploadRenderProxyIds",
               "failed to map proxy ids buffer");

  for (usize i{0}; i < active_proxy_ids.GetSize(); ++i) {
    memory[i] = active_proxy_ids[i];
  }

  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
}

void RenderProxyHandler::UploadMatrixPalettes(FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::UploadMatrixPalettes");

  if (current_skinning_matrix_count_ == 0 || skinning_matrices_.IsEmpty()) {
    return;
  }

  const auto size{static_cast<GLsizei>(current_skinning_matrix_count_ *
                                       sizeof(math::Mat4))};

  EnsureStorageBufferCapacity(staging_ssbo_matrix_palettes_[frame_index],
                              staging_ssbo_matrix_palettes_sizes_[frame_index],
                              size, GL_SHADER_STORAGE_BUFFER,
                              "staging_ssbo_matrix_palettes_");

  EnsureStorageBufferCapacity(ssbo_matrix_palettes_[frame_index],
                              ssbo_matrix_palettes_sizes_[frame_index], size,
                              GL_SHADER_STORAGE_BUFFER,
                              "ssbo_matrix_palettes_");

  UploadStorageBuffer(ssbo_matrix_palettes_[frame_index], size,
                      skinning_matrices_.GetData(), GL_SHADER_STORAGE_BUFFER);
}

void RenderProxyHandler::PrepareRenderProxyDrawData(
    FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::PrepareRenderProxyDrawData");
  ReallocateRenderProxyDrawBuffers(frame_index);
  PopulateRenderProxyDrawData(frame_index);
}

void RenderProxyHandler::ReallocateRenderProxyDrawBuffers(
    FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::ReallocateRenderProxyDrawBuffers");

  if (indirect_batches_.IsEmpty() || batch_entries_.IsEmpty()) {
    return;
  }

  const auto indirect_size{static_cast<GLsizei>(
      indirect_batches_.GetSize() * sizeof(GpuIndirectRenderProxy))};

  const auto instances_size{static_cast<GLsizei>(
      batch_entries_.GetSize() * sizeof(GpuRenderProxyInstance))};

  EnsureStorageBufferCapacity(staging_ssbo_indirect_proxies_[frame_index],
                              staging_ssbo_indirect_proxies_sizes_[frame_index],
                              indirect_size, GL_SHADER_STORAGE_BUFFER,
                              "staging_ssbo_indirect_proxies_");

  EnsureStorageBufferCapacity(ssbo_indirect_proxies_[frame_index],
                              ssbo_indirect_proxies_sizes_[frame_index],
                              indirect_size, GL_DRAW_INDIRECT_BUFFER,
                              "ssbo_indirect_proxies_");

  EnsureStorageBufferCapacity(staging_ssbo_proxy_instances_[frame_index],
                              staging_ssbo_proxy_instances_sizes_[frame_index],
                              instances_size, GL_SHADER_STORAGE_BUFFER,
                              "staging_ssbo_proxy_instances_");

  EnsureStorageBufferCapacity(ssbo_proxy_instances_[frame_index],
                              ssbo_proxy_instances_sizes_[frame_index],
                              instances_size, GL_SHADER_STORAGE_BUFFER,
                              "ssbo_proxy_instances_");
}

void RenderProxyHandler::PopulateRenderProxyDrawData(
    FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::PopulateRenderProxyDrawData");

  if (indirect_batches_.IsEmpty()) {
    return;
  }

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssbo_indirect_proxies_[frame_index]);
  auto* indirect_proxies_memory{static_cast<GpuIndirectRenderProxy*>(
      glMapBuffer(GL_DRAW_INDIRECT_BUFFER, GL_WRITE_ONLY))};
  COMET_ASSERT(indirect_proxies_memory != nullptr,
               "RenderProxyHandler::PopulateRenderProxyDrawData",
               "failed to map indirect proxy buffer");

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_proxy_instances_[frame_index]);
  auto* proxy_instances_memory{static_cast<GpuRenderProxyInstance*>(
      glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY))};
  COMET_ASSERT(proxy_instances_memory != nullptr,
               "RenderProxyHandler::PopulateRenderProxyDrawData",
               "failed to map proxy instance buffer");

  usize proxy_instance_index{0};

  for (usize batch_id{0}; batch_id < indirect_batches_.GetSize(); ++batch_id) {
    PopulateRenderIndirectProxy(static_cast<BatchId>(batch_id),
                                indirect_proxies_memory);
    PopulateProxyInstances(static_cast<BatchId>(batch_id),
                           proxy_instances_memory, proxy_instance_index);
  }

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssbo_indirect_proxies_[frame_index]);
  glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_proxy_instances_[frame_index]);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, kInvalidGlNativeStorageHandle);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
}

void RenderProxyHandler::PopulateRenderIndirectProxy(
    BatchId batch_id, GpuIndirectRenderProxy* memory) {
  auto& batch{indirect_batches_[batch_id]};
  const auto& record{render_proxy_record_store_->GetRecord(batch.proxy_id)};

  auto& indirect_proxy{memory[batch_id]};
  indirect_proxy = {};

  const auto* mesh_proxy{mesh_handler_->Get(record.proxy.mesh_handle)};

  COMET_ASSERT(mesh_proxy != nullptr,
               "RenderProxyHandler::PopulateRenderIndirectProxy",
               "mesh proxy is null", "batch_id", batch_id, "proxy_id",
               batch.proxy_id, "mesh_handle", record.proxy.mesh_handle);

  indirect_proxy.command.firstInstance = batch.offset;
  indirect_proxy.command.instanceCount = 0;
  indirect_proxy.command.vertexOffset = mesh_proxy->vertex_offset;
  indirect_proxy.command.firstIndex = mesh_proxy->index_offset;
  indirect_proxy.command.indexCount = mesh_proxy->index_count;
  indirect_proxy.proxy_id = batch.proxy_id;
  indirect_proxy.batch_id = batch_id;
}

void RenderProxyHandler::PopulateShadowRenderIndirectProxy(
    BatchId batch_id, GpuIndirectRenderProxy* memory) {
  auto& batch{indirect_batches_[batch_id]};
  const auto& record{render_proxy_record_store_->GetRecord(batch.proxy_id)};
  auto& indirect_proxy{memory[batch_id]};
  indirect_proxy = {};

  const auto* mesh_proxy{mesh_handler_->Get(record.proxy.mesh_handle)};

  COMET_ASSERT(mesh_proxy != nullptr,
               "RenderProxyHandler::PopulateShadowRenderIndirectProxy",
               "mesh proxy is null", "batch_id", batch_id, "proxy_id",
               batch.proxy_id, "mesh_handle", record.proxy.mesh_handle);

  indirect_proxy.command.firstInstance = batch.offset;
  indirect_proxy.command.instanceCount = batch.count;
  indirect_proxy.command.vertexOffset = mesh_proxy->vertex_offset;
  indirect_proxy.command.firstIndex = mesh_proxy->index_offset;
  indirect_proxy.command.indexCount = mesh_proxy->index_count;
  indirect_proxy.proxy_id = batch.proxy_id;
  indirect_proxy.batch_id = batch_id;
}

void RenderProxyHandler::PopulateProxyInstances(BatchId batch_id,
                                                GpuRenderProxyInstance* memory,
                                                usize& proxy_instance_index) {
  auto& batch{indirect_batches_[batch_id]};

  for (usize instance_index{0}; instance_index < batch.count;
       ++instance_index) {
    memory[proxy_instance_index].proxy_id =
        batch_entries_[instance_index + batch.offset].proxy_id;
    memory[proxy_instance_index].batch_id = static_cast<BatchId>(batch_id);
    ++proxy_instance_index;
  }
}

#ifdef COMET_DEBUG
void RenderProxyHandler::AssertRuntimeInvariants(
    FrameInFlightIndex frame_index) const {
  for (const auto proxy_id : render_proxy_record_store_->GetActiveProxyIds()) {
    COMET_ASSERT(proxy_id < proxy_material_handles_.GetSize(),
                 "RenderProxyHandler::AssertRuntimeInvariants",
                 "material handle index out of range", "proxy_id", proxy_id,
                 "material_handle_count", proxy_material_handles_.GetSize());

    COMET_ASSERT(proxy_material_handles_[proxy_id],
                 "RenderProxyHandler::AssertRuntimeInvariants",
                 "material handle is invalid", "proxy_id", proxy_id);
  }

  COMET_ASSERT(frame_index < ssbo_proxy_local_datas_.GetSize(),
               "RenderProxyHandler::AssertRuntimeInvariants",
               "frame index out of bounds", "frame_index", frame_index,
               "buffer_count", ssbo_proxy_local_datas_.GetSize());

  COMET_ASSERT(
      ssbo_proxy_local_datas_[frame_index] != kInvalidGlNativeStorageHandle,
      "RenderProxyHandler::AssertRuntimeInvariants",
      "proxy local data buffer handle is invalid", "frame_index", frame_index);

  COMET_ASSERT(ssbo_proxy_ids_[frame_index] != kInvalidGlNativeStorageHandle,
               "RenderProxyHandler::AssertRuntimeInvariants",
               "proxy ids buffer handle is invalid", "frame_index",
               frame_index);

  COMET_ASSERT(
      ssbo_proxy_instances_[frame_index] != kInvalidGlNativeStorageHandle,
      "RenderProxyHandler::AssertRuntimeInvariants",
      "proxy instances buffer handle is invalid", "frame_index", frame_index);

  COMET_ASSERT(
      ssbo_indirect_proxies_[frame_index] != kInvalidGlNativeStorageHandle,
      "RenderProxyHandler::AssertRuntimeInvariants",
      "indirect proxies buffer handle is invalid", "frame_index", frame_index);

  COMET_ASSERT(
      ssbo_matrix_palettes_[frame_index] != kInvalidGlNativeStorageHandle,
      "RenderProxyHandler::AssertRuntimeInvariants",
      "matrix palettes buffer handle is invalid", "frame_index", frame_index);
}

void RenderProxyHandler::AssertShadowCullRuntimeInvariants(
    FrameInFlightIndex frame_index) const {
  COMET_ASSERT(
      ssbo_shadow_proxy_ids_[frame_index] != kInvalidGlNativeStorageHandle,
      "RenderProxyHandler::AssertShadowCullRuntimeInvariants",
      "shadow proxy ids buffer handle is invalid", "frame_index", frame_index);

  COMET_ASSERT(ssbo_shadow_indirect_proxies_[frame_index] !=
                   kInvalidGlNativeStorageHandle,
               "RenderProxyHandler::AssertShadowCullRuntimeInvariants",
               "shadow indirect buffer handle is invalid", "frame_index",
               frame_index);
}
#endif  // COMET_DEBUG

void RenderProxyHandler::InitializeBuffers() {
  const auto max_frames_in_flight{frame_state_->GetMaxFramesInFlight()};

  const auto init_handle_array{[&](auto& handles) {
    handles = Array<GlNativeStorageHandle>::WithCapacity(&platform_allocator_,
                                                         max_frames_in_flight);
    handles.Resize(max_frames_in_flight);
    for (auto& handle : handles) {
      handle = kInvalidGlNativeStorageHandle;
    }
  }};

  const auto init_size_array{[&](auto& sizes) {
    sizes = Array<GLsizei>::WithCapacity(&platform_allocator_,
                                         max_frames_in_flight);
    sizes.Resize(max_frames_in_flight);
    for (auto& size : sizes) {
      size = 0;
    }
  }};

  init_handle_array(proxy_local_data_staging_buffers_);
  init_handle_array(sparse_upload_source_word_buffers_);
  init_handle_array(ssbo_proxy_local_datas_);

  init_handle_array(staging_ssbo_proxy_ids_);
  init_handle_array(ssbo_proxy_ids_);

  init_handle_array(staging_ssbo_matrix_palettes_);
  init_handle_array(ssbo_matrix_palettes_);

  init_handle_array(ssbo_sparse_upload_word_indices_);

  init_handle_array(staging_ssbo_indirect_proxies_);
  init_handle_array(ssbo_indirect_proxies_);

  init_handle_array(staging_ssbo_shadow_indirect_proxies_);
  init_handle_array(ssbo_shadow_indirect_proxies_);

  init_handle_array(staging_ssbo_proxy_instances_);
  init_handle_array(ssbo_proxy_instances_);

  init_handle_array(staging_ssbo_shadow_proxy_ids_);
  init_handle_array(ssbo_shadow_proxy_ids_);

  init_size_array(proxy_local_data_staging_buffer_sizes_);
  init_size_array(sparse_upload_source_word_buffer_sizes_);
  init_size_array(ssbo_proxy_local_datas_sizes_);

  init_size_array(staging_ssbo_proxy_ids_sizes_);
  init_size_array(ssbo_proxy_ids_sizes_);

  init_size_array(staging_ssbo_matrix_palettes_sizes_);
  init_size_array(ssbo_matrix_palettes_sizes_);

  init_size_array(ssbo_sparse_upload_word_indices_sizes_);

  init_size_array(staging_ssbo_indirect_proxies_sizes_);
  init_size_array(ssbo_indirect_proxies_sizes_);

  init_size_array(staging_ssbo_shadow_indirect_proxies_sizes_);
  init_size_array(ssbo_shadow_indirect_proxies_sizes_);

  init_size_array(staging_ssbo_proxy_instances_sizes_);
  init_size_array(ssbo_proxy_instances_sizes_);

  init_size_array(staging_ssbo_shadow_proxy_ids_sizes_);
  init_size_array(ssbo_shadow_proxy_ids_sizes_);

  pending_local_data_patch_proxy_ids_ =
      Array<OrderedSet<RenderProxyId>>::WithCapacity(&platform_allocator_,
                                                     max_frames_in_flight);
  pending_local_data_patch_proxy_ids_.Resize(max_frames_in_flight);

  for (auto& pending_ids : pending_local_data_patch_proxy_ids_) {
    pending_ids = OrderedSet<RenderProxyId>{&platform_allocator_};
    pending_ids.Reserve(kDefaultProxyCount_);
  }
}

void RenderProxyHandler::DestroyBuffers() {
  const auto destroy_handle_array{[](auto& handles, auto& sizes) {
    for (usize i{0}; i < handles.GetSize(); ++i) {
      if (handles[i] != kInvalidGlNativeStorageHandle) {
        glDeleteBuffers(1, &handles[i]);
        handles[i] = kInvalidGlNativeStorageHandle;
        sizes[i] = 0;
      }
    }

    handles.Release();
    sizes.Release();
  }};

  destroy_handle_array(proxy_local_data_staging_buffers_,
                       proxy_local_data_staging_buffer_sizes_);
  destroy_handle_array(sparse_upload_source_word_buffers_,
                       sparse_upload_source_word_buffer_sizes_);
  destroy_handle_array(ssbo_proxy_local_datas_, ssbo_proxy_local_datas_sizes_);

  destroy_handle_array(staging_ssbo_proxy_ids_, staging_ssbo_proxy_ids_sizes_);
  destroy_handle_array(ssbo_proxy_ids_, ssbo_proxy_ids_sizes_);

  destroy_handle_array(staging_ssbo_matrix_palettes_,
                       staging_ssbo_matrix_palettes_sizes_);
  destroy_handle_array(ssbo_matrix_palettes_, ssbo_matrix_palettes_sizes_);

  destroy_handle_array(ssbo_sparse_upload_word_indices_,
                       ssbo_sparse_upload_word_indices_sizes_);

  destroy_handle_array(staging_ssbo_indirect_proxies_,
                       staging_ssbo_indirect_proxies_sizes_);
  destroy_handle_array(ssbo_indirect_proxies_, ssbo_indirect_proxies_sizes_);

  destroy_handle_array(staging_ssbo_shadow_indirect_proxies_,
                       staging_ssbo_shadow_indirect_proxies_sizes_);
  destroy_handle_array(ssbo_shadow_indirect_proxies_,
                       ssbo_shadow_indirect_proxies_sizes_);

  destroy_handle_array(staging_ssbo_proxy_instances_,
                       staging_ssbo_proxy_instances_sizes_);
  destroy_handle_array(ssbo_proxy_instances_, ssbo_proxy_instances_sizes_);

  destroy_handle_array(staging_ssbo_shadow_proxy_ids_,
                       staging_ssbo_shadow_proxy_ids_sizes_);
  destroy_handle_array(ssbo_shadow_proxy_ids_, ssbo_shadow_proxy_ids_sizes_);

  for (auto& pending_ids : pending_local_data_patch_proxy_ids_) {
    pending_ids.Release();
  }

  pending_local_data_patch_proxy_ids_.Release();
}

void RenderProxyHandler::QueueLocalDataPatchForAllFrames(
    RenderProxyId proxy_id) {
  for (auto& pending_ids : pending_local_data_patch_proxy_ids_) {
    pending_ids.Add(proxy_id);
  }
}

void RenderProxyHandler::QueueFrameLocalDataPatches() {
  for (const auto proxy_id : render_proxy_record_store_->GetDirtyProxyIds()) {
    QueueLocalDataPatchForAllFrames(proxy_id);
  }

  for (const auto proxy_id : render_proxy_record_store_->GetRemovedProxyIds()) {
    QueueLocalDataPatchForAllFrames(proxy_id);
  }
}

void RenderProxyHandler::DestroyLiveProxyMaterials() {
  for (auto& material_handle : proxy_material_handles_) {
    if (!material_handle) {
      continue;
    }

    material_handler_->Destroy(material_handle);
    material_handle.Invalidate();
  }
}

void RenderProxyHandler::EnsureStorageBufferCapacity(
    GlNativeStorageHandle& handle, GLsizei& capacity, GLsizei required_size,
    GLenum target, [[maybe_unused]] const schar* debug_label) {
  COMET_ASSERT(required_size >= 0,
               "RenderProxyHandler::EnsureStorageBufferCapacity",
               "required size is negative", "required_size", required_size);

  if (handle == kInvalidGlNativeStorageHandle) {
    glCreateBuffers(1, &handle);
    COMET_ASSERT(handle != kInvalidGlNativeStorageHandle,
                 "RenderProxyHandler::EnsureStorageBufferCapacity",
                 "failed to create storage buffer", "debug_label", debug_label);
    COMET_GL_SET_STORAGE_DEBUG_LABEL(handle, debug_label);
  }

  if (required_size <= capacity) {
    return;
  }

  glBindBuffer(target, handle);
  glBufferData(target, required_size, nullptr, GL_DYNAMIC_DRAW);
  glBindBuffer(target, kInvalidGlNativeStorageHandle);

  capacity = required_size;
}

void RenderProxyHandler::UploadStorageBuffer(GlNativeStorageHandle handle,
                                             GLsizei size, const void* data,
                                             GLenum target) {
  COMET_ASSERT(handle != kInvalidGlNativeStorageHandle,
               "RenderProxyHandler::UploadStorageBuffer",
               "storage buffer handle is invalid");
  COMET_ASSERT(size > 0, "RenderProxyHandler::UploadStorageBuffer",
               "storage buffer upload size is zero");
  COMET_ASSERT(data != nullptr, "RenderProxyHandler::UploadStorageBuffer",
               "storage buffer upload data is null");

  glBindBuffer(target, handle);
  auto* memory{glMapBuffer(target, GL_WRITE_ONLY)};
  COMET_ASSERT(memory != nullptr, "RenderProxyHandler::UploadStorageBuffer",
               "failed to map storage buffer", "size", size);

  memory::CopyMemory(memory, data, size);

  glUnmapBuffer(target);
  glBindBuffer(target, kInvalidGlNativeStorageHandle);
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet