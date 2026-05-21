// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/handler/vulkan_render_proxy_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/algorithm/sort.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/ordered_set.h"
#include "comet/math/matrix.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_mesh.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/type/shader.h"

namespace comet {
namespace rendering {
namespace vk {
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

  const auto frame_index{context_->GetFrameInFlightIndex()};

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

  auto& frame_data{context_->GetFrameData(frame_index)};

  ApplyBufferMemoryBarriers(*post_update_barriers_,
                            frame_data.command_buffer_handle,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                                VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);

#ifdef COMET_DEBUG
  AssertRuntimeInvariants(frame_index);
#endif  // COMET_DEBUG

  update_frame_ = context_->GetFrameCount();
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

  const auto& proxy_local_datas{ssbo_proxy_local_datas_[frame_index]};
  const auto& proxy_ids{ssbo_proxy_ids_[frame_index]};
  const auto& matrix_palettes{ssbo_matrix_palettes_[frame_index]};
  const auto& proxy_instances{ssbo_proxy_instances_[frame_index]};
  const auto& indirect_proxies{ssbo_indirect_proxies_[frame_index]};

  gpu_data.ssbo_proxy_local_datas_handle = proxy_local_datas.handle;
  gpu_data.ssbo_proxy_local_datas_size = proxy_local_datas.size;

  gpu_data.ssbo_proxy_ids_handle = proxy_ids.handle;
  gpu_data.ssbo_proxy_ids_size = proxy_ids.size;

  gpu_data.ssbo_matrix_palettes_handle = matrix_palettes.handle;
  gpu_data.ssbo_matrix_palettes_size = matrix_palettes.size;

  gpu_data.ssbo_proxy_instances_handle = proxy_instances.handle;
  gpu_data.ssbo_proxy_instances_size = proxy_instances.size;

  gpu_data.ssbo_indirect_proxies_handle = indirect_proxies.handle;
  gpu_data.ssbo_indirect_proxies_size = indirect_proxies.size;

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
      ssbo_sparse_upload_word_indices_[frame_index].handle;
  gpu_data.ssbo_sparse_upload_word_indices_size =
      ssbo_sparse_upload_word_indices_[frame_index].size;

  gpu_data.ssbo_source_words_handle =
      sparse_upload_source_word_buffers_[frame_index].handle;
  gpu_data.ssbo_source_words_size =
      sparse_upload_source_word_buffers_[frame_index].size;

  gpu_data.ssbo_destination_words_handle =
      ssbo_proxy_local_datas_[frame_index].handle;
  gpu_data.ssbo_destination_words_size =
      ssbo_proxy_local_datas_[frame_index].size;

  gpu_data.word_count = sparse_patch_word_counts_[frame_index];
  return gpu_data;
}

void RenderProxyHandler::PopulateSparseUploadReadBarriers(
    FrameInFlightIndex frame_index, Array<VkBufferMemoryBarrier>& out) const {
  if (!HasPendingSparseUpload(frame_index)) {
    return;
  }

  COMET_ASSERT(frame_index < ssbo_sparse_upload_word_indices_.GetSize(),
               "RenderProxyHandler::PopulateSparseUploadReadBarriers",
               "frame index out of bounds for sparse upload word indices",
               "frame_index", frame_index, "buffer_count",
               ssbo_sparse_upload_word_indices_.GetSize());

  COMET_ASSERT(frame_index < sparse_upload_source_word_buffers_.GetSize(),
               "RenderProxyHandler::PopulateSparseUploadReadBarriers",
               "frame index out of bounds for local data upload buffers",
               "frame_index", frame_index, "buffer_count",
               sparse_upload_source_word_buffers_.GetSize());

  const auto& word_indices_buffer{
      ssbo_sparse_upload_word_indices_[frame_index]};
  const auto& source_words_buffer{
      sparse_upload_source_word_buffers_[frame_index]};

  if (word_indices_buffer.handle != VK_NULL_HANDLE) {
    AddBufferMemoryBarrier(word_indices_buffer, &out, VK_ACCESS_HOST_WRITE_BIT,
                           VK_ACCESS_SHADER_READ_BIT);
  }

  if (source_words_buffer.handle != VK_NULL_HANDLE) {
    AddBufferMemoryBarrier(source_words_buffer, &out, VK_ACCESS_HOST_WRITE_BIT,
                           VK_ACCESS_SHADER_READ_BIT);
  }
}

void RenderProxyHandler::PopulateSparseUploadBarriers(
    FrameInFlightIndex frame_index, Array<VkBufferMemoryBarrier>& out) const {
  if (!HasPendingSparseUpload(frame_index)) {
    return;
  }

  AddBufferMemoryBarrier(ssbo_proxy_local_datas_[frame_index], &out,
                         VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
}

u32 RenderProxyHandler::GetCullGroupCount() const noexcept {
  return static_cast<u32>((batch_entries_.GetSize() + kShaderLocalSize - 1) /
                          kShaderLocalSize);
}

void RenderProxyHandler::PopulateDrawCullBarriers(
    FrameInFlightIndex frame_index, Array<VkBufferMemoryBarrier>& out) const {
  if (batch_entries_.IsEmpty()) {
    return;
  }

  AddBufferMemoryBarrier(ssbo_proxy_ids_[frame_index], &out,
                         VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

  const auto& indirect{ssbo_indirect_proxies_[frame_index]};

  AddBufferMemoryBarrier(
      indirect, &out, VK_ACCESS_SHADER_WRITE_BIT,
      VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_SHADER_READ_BIT);
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

  const auto indirect_size{static_cast<VkDeviceSize>(
      shadow_job_count * batch_count * sizeof(GpuIndirectRenderProxy))};
  const auto proxy_ids_size{static_cast<VkDeviceSize>(
      shadow_job_count * instance_count * sizeof(RenderProxyId))};

  auto* allocator{context_->GetAllocatorHandle()};
  auto& frame_data{context_->GetFrameData(frame_index)};

  auto& staging_indirect{staging_ssbo_shadow_indirect_proxies_[frame_index]};
  auto& gpu_indirect{ssbo_shadow_indirect_proxies_[frame_index]};
  auto& gpu_proxy_ids{ssbo_shadow_proxy_ids_[frame_index]};

  {
    const auto result{EnsureBufferCapacity(
        staging_indirect, frame_data.upload_command_buffer_handle, allocator,
        indirect_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE, false,
        "staging_ssbo_shadow_indirect_proxies_")};

    if (result.old_buffer.handle != VK_NULL_HANDLE) {
      pending_buffer_destroys_[frame_index].PushLast(result.old_buffer);
    }
  }

  {
    const auto result{EnsureBufferCapacity(
        gpu_indirect, frame_data.upload_command_buffer_handle, allocator,
        indirect_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE, false,
        "ssbo_shadow_indirect_proxies_")};

    if (result.old_buffer.handle != VK_NULL_HANDLE) {
      pending_buffer_destroys_[frame_index].PushLast(result.old_buffer);
    }
  }

  {
    const auto result{EnsureBufferCapacity(
        gpu_proxy_ids, frame_data.upload_command_buffer_handle, allocator,
        proxy_ids_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE, false,
        "ssbo_shadow_proxy_ids_")};

    if (result.old_buffer.handle != VK_NULL_HANDLE) {
      pending_buffer_destroys_[frame_index].PushLast(result.old_buffer);
    }
  }

  {
    ScopedMappedBuffer mapped{staging_indirect};
    auto* memory{
        static_cast<GpuIndirectRenderProxy*>(staging_indirect.mapped_memory)};

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
  }

  VkBufferCopy indirect_copy{};
  indirect_copy.size = indirect_size;

  vkCmdCopyBuffer(frame_data.upload_command_buffer_handle,
                  staging_indirect.handle, gpu_indirect.handle, 1,
                  &indirect_copy);

  AddUploadReleaseBarrier(frame_index, gpu_indirect);

  frame_data.has_upload_submission = true;
  frame_data.requires_upload_ownership_acquire |=
      !context_->GetDevice().IsUploadQueueGraphics();

  const auto& upload_queue{context_->GetDevice().GetUploadQueueContext()};
  const auto& graphics_queue{context_->GetDevice().GetGraphicsQueueContext()};

  AddBufferMemoryBarrier(
      gpu_indirect, post_update_barriers_, VK_ACCESS_NONE,
      VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
          VK_ACCESS_SHADER_WRITE_BIT,
      upload_queue.family_index, graphics_queue.family_index);

  AddBufferMemoryBarrier(gpu_proxy_ids, post_update_barriers_, VK_ACCESS_NONE,
                         VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                         upload_queue.family_index,
                         graphics_queue.family_index);

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

void RenderProxyHandler::PopulateShadowCullBarriers(
    FrameInFlightIndex frame_index, Array<VkBufferMemoryBarrier>& out) const {
  AddBufferMemoryBarrier(ssbo_shadow_proxy_ids_[frame_index], &out,
                         VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

  AddBufferMemoryBarrier(
      ssbo_shadow_indirect_proxies_[frame_index], &out,
      VK_ACCESS_SHADER_WRITE_BIT,
      VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_SHADER_READ_BIT);
}

const Array<RenderBatchGroup>* RenderProxyHandler::GetBatchGroups()
    const noexcept {
  return &batch_groups_;
}

const Array<RenderIndirectBatch>* RenderProxyHandler::GetIndirectBatches()
    const noexcept {
  return &indirect_batches_;
}

const Buffer& RenderProxyHandler::GetIndirectBuffer(
    FrameInFlightIndex frame_index) const noexcept {
  return ssbo_indirect_proxies_[frame_index];
}

const Buffer& RenderProxyHandler::GetShadowIndirectBuffer(
    FrameInFlightIndex frame_index) const noexcept {
  return ssbo_shadow_indirect_proxies_[frame_index];
}

const Buffer& RenderProxyHandler::GetShadowProxyIdsBuffer(
    FrameInFlightIndex frame_index) const noexcept {
  return ssbo_shadow_proxy_ids_[frame_index];
}

void RenderProxyHandler::ReleasePendingUploadResources(
    FrameInFlightIndex frame_index) {
  auto& buffers{pending_buffer_destroys_[frame_index]};

  for (auto& buffer : buffers) {
    if (IsBufferInitialized(buffer)) {
      DestroyBuffer(buffer);
    }
  }

  buffers.Clear();
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

  sparse_patch_word_counts_ = Array<u32>::WithCapacity(
      &platform_allocator_, context_->GetMaxFramesInFlight());
  sparse_patch_word_counts_.Resize(context_->GetMaxFramesInFlight());

  for (usize i{0}; i < sparse_patch_word_counts_.GetSize(); ++i) {
    sparse_patch_word_counts_[i] = 0;
  }

  render_proxy_count_ = 0;
  render_proxy_visible_count_ = 0;
  current_skinning_matrix_count_ = 0;
  update_frame_ = kInvalidFrameIndex;

  InitializeBuffers();

  pending_buffer_destroys_ = Array<Array<Buffer>>::WithCapacity(
      &platform_allocator_,
      static_cast<usize>(context_->GetMaxFramesInFlight()));
  pending_buffer_destroys_.Resize(context_->GetMaxFramesInFlight());

  for (usize i{0}; i < pending_buffer_destroys_.GetSize(); ++i) {
    pending_buffer_destroys_[i] = Array<Buffer>{&platform_allocator_};
  }
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

  update_frame_ = kInvalidFrameIndex;
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
  post_update_barriers_ = COMET_FRAME_ARRAY_WITH_CAPACITY(
      VkBufferMemoryBarrier, kDefaultUpdateBarrierCapacity_);

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
  post_update_barriers_ = nullptr;
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

  const auto frame_index{context_->GetFrameInFlightIndex()};

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
  COMET_ASSERT(frame_index < context_->GetMaxFramesInFlight(),
               "RenderProxyHandler::EnsureFrameBuffersInitialized",
               "frame index out of bounds", "frame_index", frame_index,
               "max_frames_in_flight", context_->GetMaxFramesInFlight());

  auto* allocator{context_->GetAllocatorHandle()};

  const auto ensure_gpu_buffer{[&](Buffer& buffer, VkDeviceSize size,
                                   VkBufferUsageFlags usage,
                                   const schar* debug_name) {
    if (IsBufferInitialized(buffer)) {
      return;
    }

    RecreateBuffer(buffer, allocator, size, usage, VMA_MEMORY_USAGE_GPU_ONLY, 0,
                   0, VK_SHARING_MODE_EXCLUSIVE, debug_name);
  }};

  const auto ensure_staging_buffer{[&](Buffer& buffer, VkDeviceSize size,
                                       const schar* debug_name) {
    if (IsBufferInitialized(buffer)) {
      return;
    }

    RecreateBuffer(buffer, allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                   VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
                   debug_name);
  }};

  const auto ensure_host_visible_buffer{[&](Buffer& buffer, VkDeviceSize size,
                                            VkBufferUsageFlags usage,
                                            const schar* debug_name) {
    if (IsBufferInitialized(buffer)) {
      return;
    }

    RecreateBuffer(buffer, allocator, size, usage, VMA_MEMORY_USAGE_CPU_TO_GPU,
                   0, 0, VK_SHARING_MODE_EXCLUSIVE, debug_name);
  }};

  ensure_staging_buffer(proxy_local_data_staging_buffers_[frame_index],
                        sizeof(GpuRenderProxyLocalData),
                        "proxy_local_data_staging_buffers_init");
  ensure_gpu_buffer(
      ssbo_proxy_local_datas_[frame_index], sizeof(GpuRenderProxyLocalData),
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      "ssbo_proxy_local_datas_init");

  ensure_staging_buffer(staging_ssbo_proxy_ids_[frame_index],
                        sizeof(RenderProxyId), "staging_ssbo_proxy_ids_init");
  ensure_gpu_buffer(
      ssbo_proxy_ids_[frame_index], sizeof(RenderProxyId),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      "ssbo_proxy_ids_init");

  ensure_staging_buffer(staging_ssbo_matrix_palettes_[frame_index],
                        sizeof(math::Mat4),
                        "staging_ssbo_matrix_palettes_init");
  ensure_gpu_buffer(
      ssbo_matrix_palettes_[frame_index], sizeof(math::Mat4),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      "ssbo_matrix_palettes_init");

  ensure_staging_buffer(staging_ssbo_indirect_proxies_[frame_index],
                        sizeof(GpuIndirectRenderProxy),
                        "staging_ssbo_indirect_proxies_init");
  ensure_gpu_buffer(
      ssbo_indirect_proxies_[frame_index], sizeof(GpuIndirectRenderProxy),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
          VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
      "ssbo_indirect_proxies_init");

  ensure_staging_buffer(staging_ssbo_shadow_indirect_proxies_[frame_index],
                        sizeof(GpuIndirectRenderProxy),
                        "staging_ssbo_shadow_indirect_proxies_init");
  ensure_gpu_buffer(ssbo_shadow_indirect_proxies_[frame_index],
                    sizeof(GpuIndirectRenderProxy),
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                    "ssbo_shadow_indirect_proxies_init");

  ensure_staging_buffer(staging_ssbo_proxy_instances_[frame_index],
                        sizeof(GpuRenderProxyInstance),
                        "staging_ssbo_proxy_instances_init");
  ensure_gpu_buffer(
      ssbo_proxy_instances_[frame_index], sizeof(GpuRenderProxyInstance),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      "ssbo_proxy_instances_init");

  ensure_host_visible_buffer(ssbo_sparse_upload_word_indices_[frame_index],
                             sizeof(u32), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                             "ssbo_sparse_upload_word_indices_init");
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

  auto* allocator{context_->GetAllocatorHandle()};
  auto& frame_data{context_->GetFrameData(frame_index)};

  const auto record_slot_count{
      render_proxy_record_store_->GetRecordSlotCount()};
  const auto destination_size{static_cast<VkDeviceSize>(
      record_slot_count * sizeof(GpuRenderProxyLocalData))};

  auto& destination_buffer{ssbo_proxy_local_datas_[frame_index]};
  COMET_ASSERT(destination_buffer.size >= destination_size,
               "RenderProxyHandler::UploadSparseRenderProxyLocalData",
               "proxy local data buffer is too small for sparse upload",
               "buffer_size", destination_buffer.size, "required_size",
               destination_size);

  const auto result{EnsureBufferCapacity(
      destination_buffer, frame_data.upload_command_buffer_handle, allocator,
      destination_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE, true,
      "ssbo_proxy_local_datas_")};

  if (result.old_buffer.handle != VK_NULL_HANDLE) {
    pending_buffer_destroys_[frame_index].PushLast(result.old_buffer);
    UploadAllRenderProxyLocalData(frame_index);
    return;
  }

  if (result.has_transfer_work) {
    const auto& device{context_->GetDevice()};

    frame_data.has_upload_submission = true;
    frame_data.requires_upload_ownership_acquire |=
        !device.IsUploadQueueGraphics();

    AddUploadReleaseBarrier(frame_index, destination_buffer);

    const auto& upload_queue{device.GetUploadQueueContext()};
    const auto& graphics_queue{device.GetGraphicsQueueContext()};

    AddBufferMemoryBarrier(
        destination_buffer, post_update_barriers_, VK_ACCESS_NONE,
        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
        upload_queue.family_index, graphics_queue.family_index);
  }

  auto& word_indices_buffer{ssbo_sparse_upload_word_indices_[frame_index]};
  auto& source_words_buffer{sparse_upload_source_word_buffers_[frame_index]};

  const auto word_indices_size{
      static_cast<VkDeviceSize>(word_count * sizeof(u32))};
  const auto source_words_size{static_cast<VkDeviceSize>(
      pending_count * sizeof(GpuRenderProxyLocalData))};

  RecreateBuffer(word_indices_buffer, allocator, word_indices_size,
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
                 "ssbo_sparse_upload_word_indices_");

  RecreateBuffer(
      source_words_buffer, allocator, source_words_size,
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "sparse_upload_source_word_buffers_");

  {
    ScopedMappedBuffer mapped_indices{word_indices_buffer};
    ScopedMappedBuffer mapped_source{source_words_buffer};

    auto* word_indices{static_cast<u32*>(word_indices_buffer.mapped_memory)};
    auto* source_memory{static_cast<GpuRenderProxyLocalData*>(
        source_words_buffer.mapped_memory)};

    for (usize i{0}; i < pending_count; ++i) {
      const auto proxy_index{(*local_data_patch_proxy_indices_)[i]};

      COMET_ASSERT(proxy_index < record_slot_count,
                   "RenderProxyHandler::UploadSparseRenderProxyLocalData",
                   "proxy patch index out of bounds", "proxy_index",
                   proxy_index, "record_slot_count", record_slot_count);

      source_memory[i] = (*local_data_patch_payloads_)[i];

      const auto destination_word_base{
          static_cast<u32>(proxy_index * kLocalDataWordCount)};
      const auto source_word_base{static_cast<u32>(i * kLocalDataWordCount)};

      for (u32 word_offset{0}; word_offset < kLocalDataWordCount;
           ++word_offset) {
        const auto word_index{
            static_cast<usize>(source_word_base + word_offset)};
        word_indices[word_index] = destination_word_base + word_offset;
      }
    }
  }

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

  const auto destination_size{static_cast<VkDeviceSize>(
      record_slot_count * sizeof(GpuRenderProxyLocalData))};

  const auto& gpu{ssbo_proxy_local_datas_[frame_index]};
  const bool needs_grow{!IsBufferInitialized(gpu) ||
                        gpu.size < destination_size};

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

  auto* allocator{context_->GetAllocatorHandle()};
  const auto size{static_cast<VkDeviceSize>(record_slot_count *
                                            sizeof(GpuRenderProxyLocalData))};

  auto& staging{proxy_local_data_staging_buffers_[frame_index]};
  auto& gpu{ssbo_proxy_local_datas_[frame_index]};

  RecreateBuffer(staging, allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
                 "proxy_local_staging");

  {
    ScopedMappedBuffer mapped{staging};
    memory::Memset(staging.mapped_memory, 0, static_cast<usize>(size));

    auto* memory{static_cast<GpuRenderProxyLocalData*>(staging.mapped_memory)};
    const auto& active_proxy_ids{
        render_proxy_record_store_->GetActiveProxyIds()};

    for (const auto proxy_id : active_proxy_ids) {
      COMET_ASSERT(proxy_id < record_slot_count,
                   "RenderProxyHandler::UploadAllRenderProxyLocalData",
                   "proxy id out of range", "proxy_id", proxy_id,
                   "record_slot_count", record_slot_count);
      memory[proxy_id] =
          render_proxy_record_store_->GetRecord(proxy_id).local_data;
    }
  }

  RecreateBuffer(gpu, allocator, size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
                 "proxy_local_gpu");

  VkBufferCopy copy{};
  copy.size = size;

  auto& frame_data{context_->GetFrameData(frame_index)};

  vkCmdCopyBuffer(frame_data.upload_command_buffer_handle, staging.handle,
                  gpu.handle, 1, &copy);

  AddUploadReleaseBarrier(frame_index, gpu);

  frame_data.has_upload_submission = true;
  frame_data.requires_upload_ownership_acquire |=
      !context_->GetDevice().IsUploadQueueGraphics();

  const auto& upload_queue{context_->GetDevice().GetUploadQueueContext()};
  const auto& graphics_queue{context_->GetDevice().GetGraphicsQueueContext()};

  AddBufferMemoryBarrier(
      gpu, post_update_barriers_, VK_ACCESS_NONE,
      VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
      upload_queue.family_index, graphics_queue.family_index);

  sparse_patch_word_counts_[frame_index] = 0;
}

void RenderProxyHandler::UploadRenderProxyIds(FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::UploadRenderProxyIds");

  const auto& active_proxy_ids{render_proxy_record_store_->GetActiveProxyIds()};

  if (active_proxy_ids.IsEmpty()) {
    return;
  }

  const auto size{static_cast<VkDeviceSize>(active_proxy_ids.GetSize() *
                                            sizeof(RenderProxyId))};

  auto& staging_buffer{staging_ssbo_proxy_ids_[frame_index]};
  auto& gpu_buffer{ssbo_proxy_ids_[frame_index]};
  auto* allocator{context_->GetAllocatorHandle()};

  RecreateBuffer(staging_buffer, allocator, size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU,
                 0, 0, VK_SHARING_MODE_EXCLUSIVE, "staging_ssbo_proxy_ids_");

  {
    ScopedMappedBuffer mapped{staging_buffer};
    auto* memory{static_cast<RenderProxyId*>(staging_buffer.mapped_memory)};

    for (usize i{0}; i < active_proxy_ids.GetSize(); ++i) {
      memory[i] = active_proxy_ids[i];
    }
  }

  RecreateBuffer(
      gpu_buffer, allocator, size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_proxy_ids_");

  VkBufferCopy copy{};
  copy.size = size;

  auto& frame_data{context_->GetFrameData(frame_index)};

  vkCmdCopyBuffer(frame_data.upload_command_buffer_handle,
                  staging_buffer.handle, gpu_buffer.handle, 1, &copy);

  AddUploadReleaseBarrier(frame_index, gpu_buffer);

  frame_data.has_upload_submission = true;
  frame_data.requires_upload_ownership_acquire |=
      !context_->GetDevice().IsUploadQueueGraphics();

  const auto& upload_queue{context_->GetDevice().GetUploadQueueContext()};
  const auto& graphics_queue{context_->GetDevice().GetGraphicsQueueContext()};

  AddBufferMemoryBarrier(gpu_buffer, post_update_barriers_, VK_ACCESS_NONE,
                         VK_ACCESS_SHADER_READ_BIT, upload_queue.family_index,
                         graphics_queue.family_index);
}

void RenderProxyHandler::UploadMatrixPalettes(FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::UploadMatrixPalettes");

  auto& staging_buffer{staging_ssbo_matrix_palettes_[frame_index]};
  auto& gpu_buffer{ssbo_matrix_palettes_[frame_index]};

  if (current_skinning_matrix_count_ == 0 || skinning_matrices_.IsEmpty()) {
    return;
  }

  const auto size{static_cast<VkDeviceSize>(current_skinning_matrix_count_ *
                                            sizeof(math::Mat4))};

  auto* allocator{context_->GetAllocatorHandle()};

  RecreateBuffer(staging_buffer, allocator, size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU,
                 0, 0, VK_SHARING_MODE_EXCLUSIVE,
                 "staging_ssbo_matrix_palettes_");

  {
    ScopedMappedBuffer mapped{staging_buffer};
    memory::CopyMemory(staging_buffer.mapped_memory,
                       skinning_matrices_.GetData(), static_cast<usize>(size));
  }

  RecreateBuffer(
      gpu_buffer, allocator, size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_matrix_palettes_");

  VkBufferCopy copy{};
  copy.size = size;

  auto& frame_data{context_->GetFrameData(frame_index)};

  vkCmdCopyBuffer(frame_data.upload_command_buffer_handle,
                  staging_buffer.handle, gpu_buffer.handle, 1, &copy);

  AddUploadReleaseBarrier(frame_index, gpu_buffer);

  frame_data.has_upload_submission = true;
  frame_data.requires_upload_ownership_acquire |=
      !context_->GetDevice().IsUploadQueueGraphics();

  const auto& upload_queue{context_->GetDevice().GetUploadQueueContext()};
  const auto& graphics_queue{context_->GetDevice().GetGraphicsQueueContext()};

  AddBufferMemoryBarrier(gpu_buffer, post_update_barriers_, VK_ACCESS_NONE,
                         VK_ACCESS_SHADER_READ_BIT, upload_queue.family_index,
                         graphics_queue.family_index);
}

void RenderProxyHandler::PrepareRenderProxyDrawData(
    FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::PrepareRenderProxyDrawData");
  ReallocateRenderProxyDrawBuffers(frame_index);
  PopulateRenderProxyDrawData(frame_index);
  UploadRenderDrawData(frame_index);
}

void RenderProxyHandler::ReallocateRenderProxyDrawBuffers(
    FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::ReallocateRenderProxyDrawBuffers");

  if (indirect_batches_.IsEmpty() || batch_entries_.IsEmpty()) {
    return;
  }

  auto* allocator{context_->GetAllocatorHandle()};
  auto& frame_data{context_->GetFrameData(frame_index)};
  const auto command_buffer_handle{frame_data.upload_command_buffer_handle};

  const auto indirect_size{static_cast<VkDeviceSize>(
      indirect_batches_.GetSize() * sizeof(GpuIndirectRenderProxy))};

  const auto instances_size{static_cast<VkDeviceSize>(
      batch_entries_.GetSize() * sizeof(GpuRenderProxyInstance))};

  auto& staging_indirect{staging_ssbo_indirect_proxies_[frame_index]};
  auto& indirect{ssbo_indirect_proxies_[frame_index]};
  auto& staging_instances{staging_ssbo_proxy_instances_[frame_index]};
  auto& instances{ssbo_proxy_instances_[frame_index]};

  {
    const auto result{EnsureBufferCapacity(
        staging_indirect, command_buffer_handle, allocator, indirect_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE, false,
        "staging_ssbo_indirect_proxies_")};

    if (result.old_buffer.handle != VK_NULL_HANDLE) {
      pending_buffer_destroys_[frame_index].PushLast(result.old_buffer);
    }
  }

  {
    const auto result{EnsureBufferCapacity(
        indirect, command_buffer_handle, allocator, indirect_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE, false,
        "ssbo_indirect_proxies_")};

    if (result.old_buffer.handle != VK_NULL_HANDLE) {
      pending_buffer_destroys_[frame_index].PushLast(result.old_buffer);
    }
  }

  {
    const auto result{EnsureBufferCapacity(
        staging_instances, command_buffer_handle, allocator, instances_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE, false,
        "staging_ssbo_proxy_instances_")};

    if (result.old_buffer.handle != VK_NULL_HANDLE) {
      pending_buffer_destroys_[frame_index].PushLast(result.old_buffer);
    }
  }

  {
    const auto result{EnsureBufferCapacity(
        instances, command_buffer_handle, allocator, instances_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE, false,
        "ssbo_proxy_instances_")};

    if (result.old_buffer.handle != VK_NULL_HANDLE) {
      pending_buffer_destroys_[frame_index].PushLast(result.old_buffer);
    }
  }
}

void RenderProxyHandler::PopulateRenderProxyDrawData(
    FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::PopulateRenderProxyDrawData");

  if (indirect_batches_.IsEmpty()) {
    return;
  }

  auto& staging_indirect{staging_ssbo_indirect_proxies_[frame_index]};
  auto& staging_instances{staging_ssbo_proxy_instances_[frame_index]};

  ScopedMappedBuffer indirect_mapped{staging_indirect};
  ScopedMappedBuffer instances_mapped{staging_instances};

  auto* indirect_proxies_memory{
      static_cast<GpuIndirectRenderProxy*>(staging_indirect.mapped_memory)};

  auto* proxy_instances_memory{
      static_cast<GpuRenderProxyInstance*>(staging_instances.mapped_memory)};

  usize proxy_instance_index{0};

  for (usize batch_id{0}; batch_id < indirect_batches_.GetSize(); ++batch_id) {
    PopulateRenderIndirectProxy(static_cast<BatchId>(batch_id),
                                indirect_proxies_memory);
    PopulateProxyInstances(static_cast<BatchId>(batch_id),
                           proxy_instances_memory, proxy_instance_index);
  }
}

void RenderProxyHandler::UploadRenderDrawData(FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::UploadRenderDrawData");

  if (indirect_batches_.IsEmpty()) {
    return;
  }

  auto& staging_indirect{staging_ssbo_indirect_proxies_[frame_index]};
  auto& indirect{ssbo_indirect_proxies_[frame_index]};

  auto& staging_instances{staging_ssbo_proxy_instances_[frame_index]};
  auto& instances{ssbo_proxy_instances_[frame_index]};

  VkBufferCopy indirect_copy{};
  indirect_copy.srcOffset = 0;
  indirect_copy.dstOffset = 0;
  indirect_copy.size = staging_indirect.size;

  VkBufferCopy proxy_instances_copy{};
  proxy_instances_copy.srcOffset = 0;
  proxy_instances_copy.dstOffset = 0;
  proxy_instances_copy.size = staging_instances.size;

  auto& frame_data{context_->GetFrameData(frame_index)};
  const auto upload_command_buffer_handle{
      frame_data.upload_command_buffer_handle};

  vkCmdCopyBuffer(upload_command_buffer_handle, staging_indirect.handle,
                  indirect.handle, 1, &indirect_copy);

  vkCmdCopyBuffer(upload_command_buffer_handle, staging_instances.handle,
                  instances.handle, 1, &proxy_instances_copy);

  AddUploadReleaseBarrier(frame_index, indirect);
  AddUploadReleaseBarrier(frame_index, instances);

  frame_data.has_upload_submission = true;
  frame_data.requires_upload_ownership_acquire |=
      !context_->GetDevice().IsUploadQueueGraphics();

  const auto& upload_queue{context_->GetDevice().GetUploadQueueContext()};
  const auto& graphics_queue{context_->GetDevice().GetGraphicsQueueContext()};

  AddBufferMemoryBarrier(
      indirect, post_update_barriers_, VK_ACCESS_NONE,
      VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_SHADER_READ_BIT,
      upload_queue.family_index, graphics_queue.family_index);

  AddBufferMemoryBarrier(instances, post_update_barriers_, VK_ACCESS_NONE,
                         VK_ACCESS_SHADER_READ_BIT, upload_queue.family_index,
                         graphics_queue.family_index);
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

void RenderProxyHandler::AddUploadReleaseBarrier(FrameInFlightIndex frame_index,
                                                 const Buffer& buffer) {
  if (context_->GetDevice().IsUploadQueueGraphics()) {
    return;
  }

  auto& frame_data{context_->GetFrameData(frame_index)};
  auto* barriers{COMET_FRAME_ARRAY_WITH_CAPACITY(VkBufferMemoryBarrier, 1)};

  const auto& upload_queue{context_->GetDevice().GetUploadQueueContext()};
  const auto& graphics_queue{context_->GetDevice().GetGraphicsQueueContext()};

  AddBufferMemoryBarrier(buffer, barriers, VK_ACCESS_TRANSFER_WRITE_BIT,
                         VK_ACCESS_NONE, upload_queue.family_index,
                         graphics_queue.family_index);

  ApplyBufferMemoryBarriers(*barriers, frame_data.upload_command_buffer_handle,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
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

  COMET_ASSERT(ssbo_proxy_local_datas_[frame_index].handle != VK_NULL_HANDLE,
               "RenderProxyHandler::AssertRuntimeInvariants",
               "proxy local data buffer handle is null", "frame_index",
               frame_index);

  COMET_ASSERT(ssbo_proxy_ids_[frame_index].handle != VK_NULL_HANDLE,
               "RenderProxyHandler::AssertRuntimeInvariants",
               "proxy ids buffer handle is null", "frame_index", frame_index);

  COMET_ASSERT(ssbo_proxy_instances_[frame_index].handle != VK_NULL_HANDLE,
               "RenderProxyHandler::AssertRuntimeInvariants",
               "proxy instances buffer handle is null", "frame_index",
               frame_index);

  COMET_ASSERT(ssbo_indirect_proxies_[frame_index].handle != VK_NULL_HANDLE,
               "RenderProxyHandler::AssertRuntimeInvariants",
               "indirect proxies buffer handle is null", "frame_index",
               frame_index);

  COMET_ASSERT(ssbo_matrix_palettes_[frame_index].handle != VK_NULL_HANDLE,
               "RenderProxyHandler::AssertRuntimeInvariants",
               "matrix palettes buffer handle is null", "frame_index",
               frame_index);
}

void RenderProxyHandler::AssertShadowCullRuntimeInvariants(
    FrameInFlightIndex frame_index) const {
  COMET_ASSERT(ssbo_shadow_proxy_ids_[frame_index].handle != VK_NULL_HANDLE,
               "RenderProxyHandler::AssertShadowCullRuntimeInvariants",
               "shadow proxy ids buffer handle is null", "frame_index",
               frame_index);

  COMET_ASSERT(
      ssbo_shadow_indirect_proxies_[frame_index].handle != VK_NULL_HANDLE,
      "RenderProxyHandler::AssertShadowCullRuntimeInvariants",
      "shadow indirect buffer handle is null", "frame_index", frame_index);
}
#endif  // COMET_DEBUG

void RenderProxyHandler::InitializeBuffers() {
  const auto max_frames_in_flight{context_->GetMaxFramesInFlight()};

  proxy_local_data_staging_buffers_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);
  sparse_upload_source_word_buffers_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);
  ssbo_proxy_local_datas_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);

  staging_ssbo_proxy_ids_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);
  ssbo_proxy_ids_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);

  staging_ssbo_matrix_palettes_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);
  ssbo_matrix_palettes_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);

  ssbo_sparse_upload_word_indices_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);

  staging_ssbo_indirect_proxies_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);
  ssbo_indirect_proxies_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);
  staging_ssbo_shadow_indirect_proxies_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);
  ssbo_shadow_indirect_proxies_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);
  staging_ssbo_proxy_instances_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);
  ssbo_proxy_instances_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);

  staging_ssbo_shadow_proxy_ids_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);
  ssbo_shadow_proxy_ids_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, max_frames_in_flight);

  proxy_local_data_staging_buffers_.Resize(max_frames_in_flight);
  sparse_upload_source_word_buffers_.Resize(max_frames_in_flight);
  ssbo_proxy_local_datas_.Resize(max_frames_in_flight);

  staging_ssbo_proxy_ids_.Resize(max_frames_in_flight);
  ssbo_proxy_ids_.Resize(max_frames_in_flight);

  staging_ssbo_matrix_palettes_.Resize(max_frames_in_flight);
  ssbo_matrix_palettes_.Resize(max_frames_in_flight);

  ssbo_sparse_upload_word_indices_.Resize(max_frames_in_flight);

  staging_ssbo_indirect_proxies_.Resize(max_frames_in_flight);
  ssbo_indirect_proxies_.Resize(max_frames_in_flight);
  staging_ssbo_shadow_indirect_proxies_.Resize(max_frames_in_flight);
  ssbo_shadow_indirect_proxies_.Resize(max_frames_in_flight);
  staging_ssbo_proxy_instances_.Resize(max_frames_in_flight);
  ssbo_proxy_instances_.Resize(max_frames_in_flight);

  staging_ssbo_shadow_proxy_ids_.Resize(max_frames_in_flight);
  ssbo_shadow_proxy_ids_.Resize(max_frames_in_flight);

  pending_local_data_patch_proxy_ids_ =
      Array<OrderedSet<RenderProxyId>>::WithCapacity(
          &platform_allocator_, context_->GetMaxFramesInFlight());

  pending_local_data_patch_proxy_ids_.Resize(context_->GetMaxFramesInFlight());

  for (auto& pending_ids : pending_local_data_patch_proxy_ids_) {
    pending_ids = OrderedSet<RenderProxyId>{&platform_allocator_};
    pending_ids.Reserve(kDefaultProxyCount_);
  }
}

void RenderProxyHandler::DestroyBuffers() {
  const auto max_frames_in_flight{context_->GetMaxFramesInFlight()};

  for (auto& pending_buffers : pending_buffer_destroys_) {
    for (auto& buffer : pending_buffers) {
      if (IsBufferInitialized(buffer)) {
        DestroyBuffer(buffer);
      }
    }

    pending_buffers.Release();
  }

  pending_buffer_destroys_.Release();

  for (FrameInFlightIndex i{0}; i < max_frames_in_flight; ++i) {
    if (IsBufferInitialized(proxy_local_data_staging_buffers_[i])) {
      DestroyBuffer(proxy_local_data_staging_buffers_[i]);
    }

    if (IsBufferInitialized(sparse_upload_source_word_buffers_[i])) {
      DestroyBuffer(sparse_upload_source_word_buffers_[i]);
    }

    if (IsBufferInitialized(ssbo_proxy_local_datas_[i])) {
      DestroyBuffer(ssbo_proxy_local_datas_[i]);
    }

    if (IsBufferInitialized(staging_ssbo_proxy_ids_[i])) {
      DestroyBuffer(staging_ssbo_proxy_ids_[i]);
    }

    if (IsBufferInitialized(ssbo_proxy_ids_[i])) {
      DestroyBuffer(ssbo_proxy_ids_[i]);
    }

    if (IsBufferInitialized(staging_ssbo_matrix_palettes_[i])) {
      DestroyBuffer(staging_ssbo_matrix_palettes_[i]);
    }

    if (IsBufferInitialized(ssbo_matrix_palettes_[i])) {
      DestroyBuffer(ssbo_matrix_palettes_[i]);
    }

    if (IsBufferInitialized(ssbo_sparse_upload_word_indices_[i])) {
      DestroyBuffer(ssbo_sparse_upload_word_indices_[i]);
    }

    if (IsBufferInitialized(staging_ssbo_indirect_proxies_[i])) {
      DestroyBuffer(staging_ssbo_indirect_proxies_[i]);
    }

    if (IsBufferInitialized(ssbo_indirect_proxies_[i])) {
      DestroyBuffer(ssbo_indirect_proxies_[i]);
    }

    if (IsBufferInitialized(staging_ssbo_shadow_indirect_proxies_[i])) {
      DestroyBuffer(staging_ssbo_shadow_indirect_proxies_[i]);
    }

    if (IsBufferInitialized(ssbo_shadow_indirect_proxies_[i])) {
      DestroyBuffer(ssbo_shadow_indirect_proxies_[i]);
    }

    if (IsBufferInitialized(staging_ssbo_proxy_instances_[i])) {
      DestroyBuffer(staging_ssbo_proxy_instances_[i]);
    }

    if (IsBufferInitialized(ssbo_proxy_instances_[i])) {
      DestroyBuffer(ssbo_proxy_instances_[i]);
    }

    if (IsBufferInitialized(staging_ssbo_shadow_proxy_ids_[i])) {
      DestroyBuffer(staging_ssbo_shadow_proxy_ids_[i]);
    }

    if (IsBufferInitialized(ssbo_shadow_proxy_ids_[i])) {
      DestroyBuffer(ssbo_shadow_proxy_ids_[i]);
    }
  }

  proxy_local_data_staging_buffers_.Release();
  sparse_upload_source_word_buffers_.Release();
  ssbo_proxy_local_datas_.Release();

  staging_ssbo_proxy_ids_.Release();
  ssbo_proxy_ids_.Release();

  staging_ssbo_matrix_palettes_.Release();
  ssbo_matrix_palettes_.Release();

  ssbo_sparse_upload_word_indices_.Release();

  staging_ssbo_indirect_proxies_.Release();
  ssbo_indirect_proxies_.Release();
  staging_ssbo_shadow_indirect_proxies_.Release();
  ssbo_shadow_indirect_proxies_.Release();
  staging_ssbo_proxy_instances_.Release();
  ssbo_proxy_instances_.Release();

  staging_ssbo_shadow_proxy_ids_.Release();
  ssbo_shadow_proxy_ids_.Release();

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
}  // namespace vk
}  // namespace rendering
}  // namespace comet