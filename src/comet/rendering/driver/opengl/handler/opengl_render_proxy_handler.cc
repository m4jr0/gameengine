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

#include "comet/core/algorithm/back_insert_iterator.h"
#include "comet/core/algorithm/inplace_merge.h"
#include "comet/core/algorithm/set_difference.h"
#include "comet/core/algorithm/sort.h"
#include "comet/core/type/ordered_set.h"
#include "comet/entity/type/entity_id.h"
#include "comet/entity/utils/entity_id_utils.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/opengl_debug.h"
#include "comet/rendering/driver/opengl/type/opengl_mesh.h"
#include "comet/rendering/type/common.h"
#include "comet/rendering/type/shader.h"

namespace comet {
namespace rendering {
namespace gl {
RenderProxyHandler::RenderProxyHandler(const RenderProxyHandlerDescr& descr)
    : Handler{descr},
      material_handler_{descr.material_handler},
      mesh_handler_{descr.mesh_handler},
      shader_handler_{descr.shader_handler} {
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

  const auto frame_count{static_cast<FrameCount>(packet->frame_count)};

  if (update_frame_ == frame_count) {
    return;
  }

  sparse_upload_word_count_ = 0;

  GenerateUpdateTemporaryStructures(packet);
  ApplyRenderProxyChanges(packet);
  ProcessBatches();
  UploadRenderProxyLocalData();
  PrepareRenderProxyDrawData(frame_state_->GetFrameInFlightIndex());

  update_frame_ = frame_count;
}

void RenderProxyHandler::Reset() {
  DestroyUpdateTemporaryStructures();
  sparse_upload_word_count_ = 0;
}

u32 RenderProxyHandler::GetRenderProxyCount() const noexcept {
  return render_proxy_count_;
}

u32 RenderProxyHandler::GetVisibleCount() const noexcept {
  return render_proxy_visible_count_;
}

RenderProxyGpuData RenderProxyHandler::GetGpuData(
    FrameInFlightIndex frame_index) const noexcept {
  RenderProxyGpuData gpu_data{};

  gpu_data.ssbo_proxy_local_datas_handle = ssbo_proxy_local_datas_handle_;
  gpu_data.ssbo_proxy_local_datas_size = ssbo_proxy_local_datas_buffer_size_;

  gpu_data.ssbo_proxy_ids_handle = ssbo_proxy_ids_handle_[frame_index];
  gpu_data.ssbo_proxy_ids_size = ssbo_proxy_ids_buffer_size_[frame_index];

  gpu_data.ssbo_proxy_instances_handle =
      ssbo_proxy_instances_handle_[frame_index];
  gpu_data.ssbo_proxy_instances_size =
      ssbo_proxy_instances_buffer_size_[frame_index];

  gpu_data.ssbo_indirect_proxies_handle =
      ssbo_indirect_proxies_handle_[frame_index];
  gpu_data.ssbo_indirect_proxies_size =
      ssbo_indirect_proxies_buffer_size_[frame_index];

  gpu_data.ssbo_matrix_palettes_handle = ssbo_matrix_palettes_handle_;
  gpu_data.ssbo_matrix_palettes_size = ssbo_matrix_palettes_buffer_size_;

#ifdef COMET_DEBUG_RENDERING
  gpu_data.ssbo_debug_data_handle = ssbo_debug_data_handle_[frame_index];
  gpu_data.ssbo_debug_data_size = ssbo_debug_data_buffer_size_[frame_index];
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  gpu_data.ssbo_debug_aabbs_handle = ssbo_debug_aabbs_handle_;
  gpu_data.ssbo_debug_aabbs_size = ssbo_debug_aabbs_buffer_size_;

  gpu_data.ssbo_debug_lines_handle = ssbo_debug_lines_handle_;
  gpu_data.ssbo_debug_lines_size = ssbo_debug_lines_buffer_size_;
#endif  // COMET_DEBUG_CULLING

  return gpu_data;
}

bool RenderProxyHandler::HasPendingSparseUpload() const noexcept {
  return sparse_upload_word_count_ > 0;
}

u32 RenderProxyHandler::GetSparseUploadWordCount() const noexcept {
  return sparse_upload_word_count_;
}

u32 RenderProxyHandler::GetSparseUploadGroupCount() const noexcept {
  return static_cast<u32>((sparse_upload_word_count_ + kShaderLocalSize - 1) /
                          kShaderLocalSize);
}

RenderProxySparseUploadData RenderProxyHandler::GetSparseUploadGpuData()
    const noexcept {
  RenderProxySparseUploadData gpu_data{};

  gpu_data.ssbo_word_indices_handle = ssbo_word_indices_handle_;
  gpu_data.ssbo_word_indices_size = ssbo_word_indices_buffer_size_;

  gpu_data.ssbo_source_words_handle = staging_ssbo_proxy_local_datas_handle_;
  gpu_data.ssbo_source_words_size = staging_ssbo_proxy_local_datas_buffer_size_;

  gpu_data.ssbo_destination_words_handle = ssbo_proxy_local_datas_handle_;
  gpu_data.ssbo_destination_words_size = ssbo_proxy_local_datas_buffer_size_;

  gpu_data.word_count = sparse_upload_word_count_;
  return gpu_data;
}

u32 RenderProxyHandler::GetCullGroupCount() const noexcept {
  return static_cast<u32>((batch_entries_.GetSize() + kShaderLocalSize - 1) /
                          kShaderLocalSize);
}

#ifdef COMET_DEBUG_RENDERING
void RenderProxyHandler::PrepareCullDebugWrite(FrameInFlightIndex frame_index) {
  auto& debug_data{debug_data_[frame_index]};
  render_proxy_visible_count_ = debug_data->visible_count;
  debug_data->visible_count = 0;
}
#endif  // COMET_DEBUG_RENDERING

const Array<RenderBatchGroup>* RenderProxyHandler::GetBatchGroups()
    const noexcept {
  return batch_groups_;
}

const Array<RenderIndirectBatch>* RenderProxyHandler::GetIndirectBatches()
    const noexcept {
  return indirect_batches_;
}

GlNativeStorageHandle RenderProxyHandler::GetIndirectBufferHandle(
    FrameInFlightIndex frame_index) const noexcept {
  return ssbo_indirect_proxies_handle_[frame_index];
}

GlNativeStorageHandle RenderProxyHandler::GetShadowIndirectBufferHandle(
    FrameInFlightIndex frame_index) const noexcept {
  return ssbo_shadow_indirect_proxies_handle_[frame_index];
}

#ifdef COMET_DEBUG_CULLING
GlNativeStorageHandle RenderProxyHandler::GetDebugLineBufferHandle()
    const noexcept {
  return ssbo_debug_lines_handle_;
}

u32 RenderProxyHandler::GetDebugLineVertexCount() const noexcept {
  return render_proxy_count_ * 24;
}
#endif  // COMET_DEBUG_CULLING

void RenderProxyHandler::OnInitialize() {
  proxy_local_data_allocator_.Initialize();
  general_allocator_.Initialize();

  proxy_local_datas_ = Array<GpuRenderProxyLocalData>::WithCapacity(
      &proxy_local_data_allocator_, kDefaultProxyCount_);
  batch_entries_ = Array<RenderBatchEntry>::WithCapacity(&general_allocator_,
                                                         kDefaultProxyCount_);

  entity_id_to_proxy_id_map_ =
      Map<entity::EntityId, RenderProxyId>::WithCapacity(&general_allocator_,
                                                         kDefaultProxyCount_);
  model_to_proxies_map_ =
      Map<entity::EntityId, RenderProxyModelBindings>::WithCapacity(
          &general_allocator_, kDefaultProxyCount_);
  proxy_id_to_entity_id_map_ = Array<entity::EntityId>::WithCapacity(
      &general_allocator_, kDefaultProxyCount_);

  sparse_upload_word_count_ = 0;

  InitializeBuffers();

#ifdef COMET_DEBUG_RENDERING
  InitializeDebugData();
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  InitializeCullingDebug();
#endif  // COMET_DEBUG_CULLING
}

void RenderProxyHandler::OnShutdown() {
#ifdef COMET_DEBUG_RENDERING
  DestroyDebugData();
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  DestroyCullingDebug();
#endif  // COMET_DEBUG_CULLING

  DestroyLiveProxyMaterials();
  DestroyUpdateTemporaryStructures();
  DestroyBuffers();

  proxy_local_datas_.Release();
  batch_entries_.Release();
  proxy_id_to_entity_id_map_.Release();
  model_to_proxies_map_.Release();
  entity_id_to_proxy_id_map_.Release();

  update_frame_ = kInvalidFrameCount;
  render_proxy_count_ = 0;
  render_proxy_visible_count_ = 0;
  sparse_upload_word_count_ = 0;

  general_allocator_.Destroy();
  proxy_local_data_allocator_.Destroy();
}

bool RenderProxyHandler::OnRenderBatchSort(const RenderBatchEntry& a,
                                           const RenderBatchEntry& b) {
  if (a.sort_key != b.sort_key) {
    return a.sort_key < b.sort_key;
  }

  return a.proxy->id < b.proxy->id;
}

u64 RenderProxyHandler::GenerateRenderProxySortKey(const RenderProxy& proxy) {
  const auto material_hash{GenerateHash(proxy.material_handle)};
  const auto mesh_material_hash{
      HashCombine(material_hash, GenerateHash(proxy.mesh_handle))};
  return (static_cast<u64>(material_hash) << 32) | mesh_material_hash;
}

void RenderProxyHandler::GenerateUpdateTemporaryStructures(
    const frame::FramePacket* packet) {
  pending_proxy_ids_ = COMET_FRAME_ORDERED_SET_WITH_CAPACITY(
      RenderProxyId, packet->added_geometries->GetSize() + kDefaultProxyCount_);

  pending_proxy_local_data_ = COMET_FRAME_ARRAY_WITH_CAPACITY(
      GpuRenderProxyLocalData, pending_proxy_ids_->GetCapacity());

  pending_proxy_indices_ = COMET_FRAME_ARRAY_WITH_CAPACITY(
      usize, packet->added_geometries->GetSize() +
                 packet->removed_geometries->GetSize());

  destroyed_proxies_ = COMET_FRAME_ARRAY_WITH_CAPACITY(
      RenderProxy, packet->removed_geometries->GetSize());

  destroyed_batch_entries_ = COMET_FRAME_ARRAY_WITH_CAPACITY(
      RenderBatchEntry, packet->removed_geometries->GetSize());

  destroyed_entity_ids_ = COMET_FRAME_ORDERED_SET_WITH_CAPACITY(
      entity::EntityId, packet->removed_geometries->GetSize());

  indirect_batches_ = COMET_FRAME_ARRAY_WITH_CAPACITY(
      RenderIndirectBatch, kDefaultRenderIndirectBatchCount_);

  batch_groups_ = COMET_FRAME_ARRAY_WITH_CAPACITY(
      RenderBatchGroup, kDefaultRenderBatchGroupCount_);
}

void RenderProxyHandler::DestroyUpdateTemporaryStructures() {
  pending_proxy_ids_ = nullptr;
  pending_proxy_local_data_ = nullptr;
  pending_proxy_indices_ = nullptr;
  destroyed_proxies_ = nullptr;
  destroyed_batch_entries_ = nullptr;
  destroyed_entity_ids_ = nullptr;
  indirect_batches_ = nullptr;
  batch_groups_ = nullptr;
}

void RenderProxyHandler::ApplyRenderProxyChanges(
    const frame::FramePacket* packet) {
  COMET_PROFILE("RenderProxyHandler::ApplyRenderProxyChanges");

  const auto generated_proxy_count{packet->added_geometries->GetSize()};
  const auto destroyed_proxy_count{packet->removed_geometries->GetSize()};
  const auto resize_delta{generated_proxy_count > destroyed_proxy_count
                              ? generated_proxy_count - destroyed_proxy_count
                              : 0};

  if (resize_delta > 0) {
    proxy_id_to_entity_id_map_.Resize(proxy_id_to_entity_id_map_.GetSize() +
                                      resize_delta);
  }

  DestroyRenderProxies(packet->removed_geometries);
  GenerateRenderProxies(packet->added_geometries);
  UpdateRenderProxies(packet->dirty_meshes, packet->dirty_transforms);
  UpdateSkinningMatrices(packet->skinning_bindings, packet->matrix_palettes);
}

void RenderProxyHandler::ProcessBatches() {
  COMET_PROFILE("RenderProxyHandler::ProcessBatches");
  GenerateBatchEntries();
  GenerateIndirectBatches();
  GenerateBatchGroups();
}

void RenderProxyHandler::GenerateRenderProxies(
    const frame::AddedGeometries* geometries) {
  COMET_PROFILE("RenderProxyHandler::GenerateRenderProxies");

  const auto generated_proxy_count{geometries->GetSize()};

  if (generated_proxy_count == 0) {
    return;
  }

  for (usize i{0}; i < generated_proxy_count; ++i) {
    COMET_ASSERT(render_proxy_count_ != kMaxRenderProxyCount_,
                 "RenderProxyHandler::GenerateRenderProxies",
                 "max render proxy count reached");

    const auto& geometry{geometries->Get(i)};

    auto& local_data{proxy_local_datas_.EmplaceLast()};
    local_data.local_center = math::Vec4{geometry.local_center, .0f};
    local_data.local_max_extents = math::Vec4{geometry.local_max_extents, .0f};
    local_data.transform = geometry.transform;

    auto& new_proxy{proxies_[render_proxy_count_]};
    new_proxy.id = static_cast<RenderProxyId>(render_proxy_count_);
    new_proxy.model_entity_id = geometry.model_entity_id;
    new_proxy.mesh_handle = geometry.mesh_handle;
    new_proxy.material_handle =
        material_handler_->GetOrGenerate(geometry.material_resource_id);

    COMET_ASSERT(new_proxy.mesh_handle,
                 "RenderProxyHandler::GenerateRenderProxies",
                 "mesh handle is invalid", "entity_id", geometry.entity_id,
                 "model_entity_id", geometry.model_entity_id,
                 "material_resource_id", geometry.material_resource_id);

    entity_id_to_proxy_id_map_.Set(geometry.entity_id, new_proxy.id);
    proxy_id_to_entity_id_map_[new_proxy.id] = geometry.entity_id;

    RegisterModelProxy(geometry.model_entity_id, new_proxy.id);

    pending_proxy_ids_->Add(new_proxy.id);
    pending_proxy_indices_->PushLast(new_proxy.id);
    pending_proxy_local_data_->PushLast(local_data);

    ++render_proxy_count_;
  }
}

void RenderProxyHandler::UpdateRenderProxies(
    const frame::DirtyMeshes* meshes,
    const frame::DirtyTransforms* transforms) {
  COMET_PROFILE("RenderProxyHandler::UpdateRenderProxies");

  const auto updated_mesh_count{meshes->GetSize()};
  const auto updated_transform_count{transforms->GetSize()};

  if (updated_mesh_count == 0 && updated_transform_count == 0) {
    return;
  }

  for (usize i{0}; i < updated_mesh_count; ++i) {
    auto& updated_mesh{meshes->Get(i)};

    if (destroyed_entity_ids_->IsContained(updated_mesh.entity_id)) {
      continue;
    }

    COMET_ASSERT(entity_id_to_proxy_id_map_.IsContained(updated_mesh.entity_id),
                 "RenderProxyHandler::UpdateRenderProxies",
                 "mesh proxy does not exist", "entity_id",
                 updated_mesh.entity_id);

    const auto proxy_id{entity_id_to_proxy_id_map_.Get(updated_mesh.entity_id)};
    auto& updated_proxy{proxies_[proxy_id]};
    pending_proxy_ids_->Add(updated_proxy.id);

    auto& local_data{proxy_local_datas_[proxy_id]};
    local_data.local_center = math::Vec4{updated_mesh.local_center, .0f};
    local_data.local_max_extents =
        math::Vec4{updated_mesh.local_max_extents, .0f};

    pending_proxy_local_data_->PushLast(local_data);
  }

  for (usize i{0}; i < updated_transform_count; ++i) {
    auto& updated_transform{transforms->Get(i)};

    if (destroyed_entity_ids_->IsContained(updated_transform.entity_id)) {
      continue;
    }

    if (!entity_id_to_proxy_id_map_.IsContained(updated_transform.entity_id)) {
      continue;
    }

    const auto proxy_id{
        entity_id_to_proxy_id_map_.Get(updated_transform.entity_id)};
    auto& updated_proxy{proxies_[proxy_id]};
    pending_proxy_ids_->Add(updated_proxy.id);

    auto& local_data{proxy_local_datas_[proxy_id]};
    local_data.transform = updated_transform.transform;

    pending_proxy_local_data_->PushLast(local_data);
  }
}

void RenderProxyHandler::DestroyRenderProxies(
    const frame::RemovedGeometries* geometries) {
  COMET_PROFILE("RenderProxyHandler::DestroyRenderProxies");

  if (geometries->IsEmpty()) {
    return;
  }

  for (const auto& geometry : *geometries) {
    destroyed_entity_ids_->Add(geometry.entity_id);
    const auto proxy_id_ptr{
        entity_id_to_proxy_id_map_.TryGet(geometry.entity_id)};

    if (proxy_id_ptr == nullptr) {
      COMET_LOG_WARNING(
          LoggerType::Rendering, "RenderProxyHandler::DestroyRenderProxies",
          "render proxy not found", "entity_id", geometry.entity_id);
      continue;
    }

    const auto proxy_id{*proxy_id_ptr};
    COMET_ASSERT(proxy_id < render_proxy_count_,
                 "RenderProxyHandler::DestroyRenderProxies",
                 "proxy id out of range", "proxy_id", proxy_id,
                 "render_proxy_count", render_proxy_count_, "entity_id",
                 geometry.entity_id);

    material_handler_->Destroy(proxies_[proxy_id].material_handle);

    auto& destroyed_proxy{destroyed_proxies_->EmplaceLast(proxies_[proxy_id])};

    RenderBatchEntry destroyed_batch_entry{};
    destroyed_batch_entry.proxy = &destroyed_proxy;
    destroyed_batch_entry.sort_key =
        GenerateRenderProxySortKey(*destroyed_batch_entry.proxy);
    destroyed_batch_entries_->PushLast(destroyed_batch_entry);
    UnregisterModelProxy(geometry.model_entity_id, proxy_id);

    const auto old_proxy_id{
        static_cast<RenderProxyId>(render_proxy_count_ - 1)};

    if (proxy_id != old_proxy_id) {
      proxies_[proxy_id] = proxies_[old_proxy_id];
      proxy_local_datas_[proxy_id] = proxy_local_datas_[old_proxy_id];
      entity_id_to_proxy_id_map_.Set(
          proxy_id_to_entity_id_map_.Get(old_proxy_id), proxy_id);

      pending_proxy_ids_->Add(proxy_id);
      pending_proxy_local_data_->PushLast(proxy_local_datas_[proxy_id]);

      const auto& new_proxy{proxies_[proxy_id]};
      UnregisterModelProxy(new_proxy.model_entity_id, old_proxy_id);
      RegisterModelProxy(new_proxy.model_entity_id, proxy_id);
    }

    entity_id_to_proxy_id_map_.Remove(geometry.entity_id);
    --render_proxy_count_;
  }

  if (destroyed_batch_entries_->IsEmpty()) {
    return;
  }

  Sort(destroyed_batch_entries_->begin(), destroyed_batch_entries_->end(),
       OnRenderBatchSort);

  auto filtered_batches{Array<RenderBatchEntry>::WithCapacity(
      &general_allocator_, batch_entries_.GetSize())};

  SetDifference(batch_entries_.begin(), batch_entries_.end(),
                destroyed_batch_entries_->begin(),
                destroyed_batch_entries_->end(), BackInserter(filtered_batches),
                OnRenderBatchSort);

  batch_entries_ = std::move(filtered_batches);
}

void RenderProxyHandler::UpdateSkinningMatrices(
    const frame::SkinningBindings* bindings,
    const frame::MatrixPalettes* palettes) {
  COMET_PROFILE("RenderProxyHandler::UpdateSkinningMatrices");

  const auto entity_count{bindings->GetSize()};
  COMET_ASSERT(entity_count == palettes->GetSize(),
               "RenderProxyHandler::UpdateSkinningMatrices",
               "binding/palette count mismatch", "binding_count", entity_count,
               "palette_count", palettes->GetSize());

  if (entity_count == 0) {
    return;
  }

  usize total_joint_count{0};

  for (usize i{0}; i < entity_count; ++i) {
    auto& binding{bindings->Get(i)};
    const auto skinning_offset{static_cast<SkinningOffset>(total_joint_count)};
    total_joint_count += binding.joint_count;

    if (binding.entity_id == entity::kInvalidEntityId) {
      continue;
    }

    auto* proxies{model_to_proxies_map_.TryGet(binding.entity_id)};

    if (proxies == nullptr) {
      continue;
    }

    for (auto& proxy_id : proxies->proxy_ids) {
      auto& local_data{proxy_local_datas_[proxy_id]};
      local_data.skinning_offset = skinning_offset;
      pending_proxy_ids_->Add(proxy_id);
      pending_proxy_local_data_->PushLast(proxy_local_datas_[proxy_id]);
    }
  }

  if (total_joint_count == 0) {
    return;
  }

  const auto skinning_matrix_size{
      static_cast<GLsizei>(total_joint_count * sizeof(math::Mat4))};

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_matrix_palettes_handle_);

  if (skinning_matrix_size > ssbo_matrix_palettes_buffer_size_) {
    glBufferData(GL_SHADER_STORAGE_BUFFER, skinning_matrix_size, nullptr,
                 GL_DYNAMIC_DRAW);
    ssbo_matrix_palettes_buffer_size_ = skinning_matrix_size;
  }

  auto* memory{
      static_cast<u8*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY))};
  COMET_ASSERT(memory != nullptr, "RenderProxyHandler::UpdateSkinningMatrices",
               "failed to map matrix palette buffer");

  sptrdiff cursor{0};

  for (usize i{0}; i < entity_count; ++i) {
    const auto& palette{palettes->Get(i)};
    const auto size{palette.skinning_matrix_count * sizeof(math::Mat4)};
    memory::CopyMemory(memory + cursor, palette.skinning_matrices, size);
    cursor += size;
  }

  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
}

void RenderProxyHandler::GenerateBatchEntries() {
  COMET_PROFILE("RenderProxyHandler::GenerateBatchEntries");

  if (pending_proxy_indices_ == nullptr) {
    return;
  }

  const auto generated_proxy_count{pending_proxy_indices_->GetSize()};

  if (generated_proxy_count == 0) {
    return;
  }

  new_batch_entries_ = Array<RenderBatchEntry>::WithCapacity(
      &general_allocator_, generated_proxy_count);

  for (const auto i : *pending_proxy_indices_) {
    auto& new_proxy{proxies_[i]};
    auto& batch{new_batch_entries_.EmplaceLast()};
    batch.sort_key = GenerateRenderProxySortKey(new_proxy);
    batch.proxy = &new_proxy;
  }

  Sort(new_batch_entries_.begin(), new_batch_entries_.end(), OnRenderBatchSort);

  const auto batch_count{batch_entries_.GetSize()};
  const auto new_batch_count{new_batch_entries_.GetSize()};

  if (batch_count > 0 && new_batch_count > 0) {
    batch_entries_.PushFromRange(new_batch_entries_);
    auto* start{batch_entries_.GetData()};
    auto* pivot{start + batch_count};
    auto* end{start + batch_entries_.GetSize()};

    InplaceMerge(start, pivot, end, OnRenderBatchSort);
  } else if (batch_count == 0) {
    batch_entries_ = std::move(new_batch_entries_);
  }
}

void RenderProxyHandler::GenerateIndirectBatches() {
  COMET_PROFILE("RenderProxyHandler::GenerateIndirectBatches");

  if (batch_entries_.IsEmpty()) {
    return;
  }

  auto* first_batch{&batch_entries_[0]};
  auto* current_indirect_batch{&indirect_batches_->EmplaceLast()};
  current_indirect_batch->offset = 0;
  current_indirect_batch->count = 1;
  current_indirect_batch->proxy = first_batch->proxy;

  usize last_batch_index{0};

  for (usize batch_id{1}; batch_id < batch_entries_.GetSize(); ++batch_id) {
    auto& batch{batch_entries_[batch_id]};
    auto* proxy{batch.proxy};
    auto& last_batch{indirect_batches_->Get(last_batch_index)};

    const auto is_same_mesh{proxy->mesh_handle ==
                            last_batch.proxy->mesh_handle};
    const auto is_same_material{proxy->material_handle ==
                                last_batch.proxy->material_handle};

    if (is_same_mesh && is_same_material) {
      ++last_batch.count;
      continue;
    }

    current_indirect_batch = &indirect_batches_->EmplaceLast();
    current_indirect_batch->offset = static_cast<u32>(batch_id);
    current_indirect_batch->count = 1;
    current_indirect_batch->proxy = proxy;
    last_batch_index = indirect_batches_->GetSize() - 1;
  }
}

void RenderProxyHandler::GenerateBatchGroups() {
  COMET_PROFILE("RenderProxyHandler::GenerateBatchGroups");

  if (indirect_batches_->IsEmpty()) {
    return;
  }

  auto* current_group{&batch_groups_->EmplaceLast()};
  current_group->offset = 0;
  current_group->count = 1;

  for (usize i{1}; i < indirect_batches_->GetSize(); ++i) {
    auto& anchor_batch{indirect_batches_->Get(current_group->offset)};
    auto& batch{indirect_batches_->Get(i)};

    if (anchor_batch.proxy->material_handle == batch.proxy->material_handle) {
      ++current_group->count;
      continue;
    }

    current_group = &batch_groups_->EmplaceLast();
    current_group->offset = static_cast<u32>(i);
    current_group->count = 1;
  }
}

void RenderProxyHandler::UploadRenderProxyLocalData() {
  COMET_PROFILE("RenderProxyHandler::UploadRenderProxyLocalData");

  if (pending_proxy_local_data_->IsEmpty()) {
    return;
  }

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};

#ifdef COMET_DEBUG_CULLING
  const auto debug_aabb_size{
      static_cast<GLsizei>(render_proxy_count_ * sizeof(GpuDebugAabb))};

  if (debug_aabb_size > ssbo_debug_aabbs_buffer_size_) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_aabbs_handle_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, debug_aabb_size, nullptr,
                 GL_DYNAMIC_DRAW);
    ssbo_debug_aabbs_buffer_size_ = debug_aabb_size;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
  }

  const auto debug_lines_size{
      static_cast<GLsizei>(render_proxy_count_ * 24 * sizeof(math::Vec4))};

  if (debug_lines_size > ssbo_debug_lines_buffer_size_) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_lines_handle_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, debug_lines_size, nullptr,
                 GL_DYNAMIC_DRAW);
    ssbo_debug_lines_buffer_size_ = debug_lines_size;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
  }
#endif  // COMET_DEBUG_CULLING

  const auto proxy_ids_size{static_cast<GLsizei>(proxy_local_datas_.GetSize() *
                                                 sizeof(RenderProxyId))};

  if (proxy_ids_size > ssbo_proxy_ids_buffer_size_[frame_index]) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_proxy_ids_handle_[frame_index]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, proxy_ids_size, nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
    ssbo_proxy_ids_buffer_size_[frame_index] = proxy_ids_size;
  }

  if (pending_proxy_local_data_->GetSize() >
      proxy_local_datas_.GetSize() * kReuploadAllLocalDataThreshold_) {
    UploadAllRenderProxyLocalData();
  } else {
    UploadPendingRenderProxyLocalData();
  }
}

void RenderProxyHandler::UploadAllRenderProxyLocalData() {
  COMET_PROFILE("RenderProxyHandler::UploadAllRenderProxyLocalData");

  const auto full_size{static_cast<GLsizei>(proxy_local_datas_.GetSize() *
                                            sizeof(GpuRenderProxyLocalData))};

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_proxy_local_datas_handle_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, full_size, nullptr, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, full_size,
                  proxy_local_datas_.GetData());
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);

  ssbo_proxy_local_datas_buffer_size_ = full_size;
}

void RenderProxyHandler::UploadPendingRenderProxyLocalData() {
  COMET_PROFILE("RenderProxyHandler::UploadPendingRenderProxyLocalData");

  const auto pending_count{pending_proxy_ids_->GetSize()};
  const auto full_size{static_cast<GLsizei>(proxy_local_datas_.GetSize() *
                                            sizeof(GpuRenderProxyLocalData))};

  if (full_size > ssbo_proxy_local_datas_buffer_size_) {
    const auto old_handle{ssbo_proxy_local_datas_handle_};
    const auto old_size{ssbo_proxy_local_datas_buffer_size_};

    glGenBuffers(1, &ssbo_proxy_local_datas_handle_);
    COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_proxy_local_datas_handle_,
                                     "ssbo_proxy_local_datas_handle_");

    glBindBuffer(GL_COPY_WRITE_BUFFER, ssbo_proxy_local_datas_handle_);
    glBufferData(GL_COPY_WRITE_BUFFER, full_size, nullptr, GL_DYNAMIC_DRAW);

    if (old_handle != kInvalidGlNativeStorageHandle) {
      glBindBuffer(GL_COPY_READ_BUFFER, old_handle);
      glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
                          old_size);
      glDeleteBuffers(1, &old_handle);
    }

    glBindBuffer(GL_COPY_READ_BUFFER, kInvalidGlNativeStorageHandle);
    glBindBuffer(GL_COPY_WRITE_BUFFER, kInvalidGlNativeStorageHandle);
    ssbo_proxy_local_datas_buffer_size_ = full_size;
  }

  const auto staging_size{
      static_cast<GLsizei>(pending_count * sizeof(GpuRenderProxyLocalData))};
  COMET_ASSERT(staging_size % sizeof(ShaderWord) == 0,
               "RenderProxyHandler::UploadPendingRenderProxyLocalData",
               "staging size is not word aligned", "staging_size", staging_size,
               "shader_word_size", sizeof(ShaderWord));

  glBindBuffer(GL_COPY_WRITE_BUFFER, staging_ssbo_proxy_local_datas_handle_);

  if (staging_size > staging_ssbo_proxy_local_datas_buffer_size_) {
    glBufferData(GL_COPY_WRITE_BUFFER, staging_size, nullptr, GL_DYNAMIC_DRAW);
    staging_ssbo_proxy_local_datas_buffer_size_ = staging_size;
  }

  auto* data_memory{static_cast<ShaderWord*>(
      glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY))};
  COMET_ASSERT(data_memory != nullptr,
               "RenderProxyHandler::UploadPendingRenderProxyLocalData",
               "failed to map staging proxy local buffer");

  memory::CopyMemory(data_memory, pending_proxy_local_data_->GetData(),
                     staging_size);
  glUnmapBuffer(GL_COPY_WRITE_BUFFER);

  const auto word_count_per_data{
      static_cast<u32>(sizeof(GpuRenderProxyLocalData) / sizeof(ShaderWord))};

  const auto word_indices_size{static_cast<GLsizei>(
      pending_count * sizeof(ShaderWord) * word_count_per_data)};

  glBindBuffer(GL_COPY_WRITE_BUFFER, ssbo_word_indices_handle_);

  if (word_indices_size > ssbo_word_indices_buffer_size_) {
    glBufferData(GL_COPY_WRITE_BUFFER, word_indices_size, nullptr,
                 GL_DYNAMIC_DRAW);
    ssbo_word_indices_buffer_size_ = word_indices_size;
  }

  auto* word_indices_memory{static_cast<ShaderWord*>(
      glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY))};
  COMET_ASSERT(word_indices_memory != nullptr,
               "RenderProxyHandler::UploadPendingRenderProxyLocalData",
               "failed to map word index buffer");

  sparse_upload_word_count_ = 0;

  for (usize i{0}; i < pending_count; ++i) {
    const auto proxy_word_offset{static_cast<ShaderWord>(
        word_count_per_data * pending_proxy_ids_->Get(i))};

    for (u32 word_index{0}; word_index < word_count_per_data; ++word_index) {
      word_indices_memory[sparse_upload_word_count_] =
          proxy_word_offset + word_index;
      ++sparse_upload_word_count_;
    }
  }

  glUnmapBuffer(GL_COPY_WRITE_BUFFER);
  glBindBuffer(GL_COPY_WRITE_BUFFER, kInvalidGlNativeStorageHandle);
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

  if (indirect_batches_->IsEmpty()) {
    return;
  }

  const auto indirect_size{static_cast<GLsizei>(
      indirect_batches_->GetSize() * sizeof(GpuIndirectRenderProxy))};
  const auto instance_size{static_cast<GLsizei>(
      batch_entries_.GetSize() * sizeof(GpuRenderProxyInstance))};

  if (indirect_size > ssbo_indirect_proxies_buffer_size_[frame_index]) {
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER,
                 ssbo_indirect_proxies_handle_[frame_index]);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, indirect_size, nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, kInvalidGlNativeStorageHandle);
    ssbo_indirect_proxies_buffer_size_[frame_index] = indirect_size;
  }

  if (instance_size > ssbo_proxy_instances_buffer_size_[frame_index]) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,
                 ssbo_proxy_instances_handle_[frame_index]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, instance_size, nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
    ssbo_proxy_instances_buffer_size_[frame_index] = instance_size;
  }

  if (indirect_size > ssbo_shadow_indirect_proxies_buffer_size_[frame_index]) {
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER,
                 ssbo_shadow_indirect_proxies_handle_[frame_index]);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, indirect_size, nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, kInvalidGlNativeStorageHandle);
    ssbo_shadow_indirect_proxies_buffer_size_[frame_index] = indirect_size;
  }
}

void RenderProxyHandler::PopulateRenderProxyDrawData(
    FrameInFlightIndex frame_index) {
  COMET_PROFILE("RenderProxyHandler::PopulateRenderProxyDrawData");

  if (indirect_batches_->IsEmpty()) {
    return;
  }

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER,
               ssbo_indirect_proxies_handle_[frame_index]);
  auto* indirect_proxies_memory{static_cast<GpuIndirectRenderProxy*>(
      glMapBuffer(GL_DRAW_INDIRECT_BUFFER, GL_WRITE_ONLY))};
  COMET_ASSERT(indirect_proxies_memory != nullptr,
               "RenderProxyHandler::PopulateRenderProxyDrawData",
               "failed to map indirect proxy buffer");

  glBindBuffer(GL_COPY_WRITE_BUFFER,
               ssbo_shadow_indirect_proxies_handle_[frame_index]);
  auto* shadow_indirect_proxies_memory{static_cast<GpuIndirectRenderProxy*>(
      glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY))};
  COMET_ASSERT(shadow_indirect_proxies_memory != nullptr,
               "RenderProxyHandler::PopulateRenderProxyDrawData",
               "failed to map shadow indirect proxy buffer");

  glBindBuffer(GL_SHADER_STORAGE_BUFFER,
               ssbo_proxy_instances_handle_[frame_index]);
  auto* proxy_instances_memory{static_cast<GpuRenderProxyInstance*>(
      glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY))};
  COMET_ASSERT(proxy_instances_memory != nullptr,
               "RenderProxyHandler::PopulateRenderProxyDrawData",
               "failed to map proxy instance buffer");

  usize proxy_instance_index{0};

  for (usize batch_id{0}; batch_id < indirect_batches_->GetSize(); ++batch_id) {
    PopulateRenderIndirectProxy(static_cast<BatchId>(batch_id),
                                indirect_proxies_memory);
    PopulateShadowRenderIndirectProxy(static_cast<BatchId>(batch_id),
                                      shadow_indirect_proxies_memory);
    PopulateProxyInstances(static_cast<BatchId>(batch_id),
                           proxy_instances_memory, proxy_instance_index);
  }

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER,
               ssbo_indirect_proxies_handle_[frame_index]);
  glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);

  glBindBuffer(GL_COPY_WRITE_BUFFER,
               ssbo_shadow_indirect_proxies_handle_[frame_index]);
  glUnmapBuffer(GL_COPY_WRITE_BUFFER);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER,
               ssbo_proxy_instances_handle_[frame_index]);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, kInvalidGlNativeStorageHandle);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
  glBindBuffer(GL_COPY_WRITE_BUFFER, kInvalidGlNativeStorageHandle);
}

void RenderProxyHandler::PopulateRenderIndirectProxy(
    BatchId batch_id, GpuIndirectRenderProxy* memory) {
  COMET_PROFILE("RenderProxyHandler::PopulateWorldIndirectProxy");

  auto& batch{indirect_batches_->Get(batch_id)};
  const auto* mesh_proxy{mesh_handler_->Get(batch.proxy->mesh_handle)};

  COMET_ASSERT(mesh_proxy != nullptr,
               "RenderProxyHandler::PopulateRenderIndirectProxy",
               "mesh proxy is null", "batch_id", batch_id, "mesh_handle",
               batch.proxy->mesh_handle);

  auto& indirect_proxy{memory[batch_id]};
  indirect_proxy.command.firstInstance = batch.offset;
  indirect_proxy.command.instanceCount = 0;
  indirect_proxy.command.vertexOffset = mesh_proxy->vertex_offset;
  indirect_proxy.command.firstIndex = mesh_proxy->index_offset;
  indirect_proxy.command.indexCount = mesh_proxy->index_count;
  indirect_proxy.proxy_id = batch.proxy->id;
  indirect_proxy.batch_id = batch_id;
}

void RenderProxyHandler::PopulateShadowRenderIndirectProxy(
    BatchId batch_id, GpuIndirectRenderProxy* memory) {
  auto& batch{indirect_batches_->Get(batch_id)};
  const auto* mesh_proxy{mesh_handler_->Get(batch.proxy->mesh_handle)};

  COMET_ASSERT(mesh_proxy != nullptr,
               "RenderProxyHandler::PopulateShadowRenderIndirectProxy",
               "mesh proxy is null", "batch_id", batch_id, "mesh_handle",
               batch.proxy->mesh_handle);

  auto& indirect_proxy{memory[batch_id]};
  indirect_proxy.command.firstInstance = batch.offset;
  indirect_proxy.command.instanceCount = batch.count;
  indirect_proxy.command.vertexOffset = mesh_proxy->vertex_offset;
  indirect_proxy.command.firstIndex = mesh_proxy->index_offset;
  indirect_proxy.command.indexCount = mesh_proxy->index_count;
  indirect_proxy.proxy_id = batch.proxy->id;
  indirect_proxy.batch_id = batch_id;
}

void RenderProxyHandler::PopulateProxyInstances(BatchId batch_id,
                                                GpuRenderProxyInstance* memory,
                                                usize& proxy_instance_index) {
  COMET_PROFILE("RenderProxyHandler::PopulateProxyInstances");
  auto& batch{indirect_batches_->Get(batch_id)};

  for (usize instance_index{0}; instance_index < batch.count;
       ++instance_index) {
    memory[proxy_instance_index].proxy_id =
        batch_entries_[instance_index + batch.offset].proxy->id;
    memory[proxy_instance_index].batch_id = static_cast<BatchId>(batch_id);
    ++proxy_instance_index;
  }
}

void RenderProxyHandler::RegisterModelProxy(entity::EntityId model_entity_id,
                                            RenderProxyId proxy_id) {
  auto* proxies{model_to_proxies_map_.TryGet(model_entity_id)};

  if (proxies == nullptr) {
    proxies =
        &model_to_proxies_map_.Emplace(model_entity_id, &general_allocator_)
             .value;
  }

  proxies->proxy_ids.PushLast(proxy_id);
}

void RenderProxyHandler::UnregisterModelProxy(entity::EntityId model_entity_id,
                                              RenderProxyId proxy_id) {
  auto* proxies{model_to_proxies_map_.TryGet(model_entity_id)};

  if (proxies == nullptr) {
    return;
  }

  proxies->proxy_ids.RemoveFromValue(proxy_id);

  if (proxies->proxy_ids.IsEmpty()) {
    model_to_proxies_map_.Remove(model_entity_id);
  }
}

void RenderProxyHandler::InitializeBuffers() {
  glCreateBuffers(1, &staging_ssbo_proxy_local_datas_handle_);
  glCreateBuffers(1, &ssbo_proxy_local_datas_handle_);
  glCreateBuffers(1, &ssbo_matrix_palettes_handle_);
  glCreateBuffers(1, &ssbo_word_indices_handle_);

  COMET_GL_SET_STORAGE_DEBUG_LABEL(staging_ssbo_proxy_local_datas_handle_,
                                   "staging_ssbo_proxy_local_datas_handle_");
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_proxy_local_datas_handle_,
                                   "ssbo_proxy_local_datas_handle_");
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_matrix_palettes_handle_,
                                   "ssbo_matrix_palettes_handle_");
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_word_indices_handle_,
                                   "ssbo_word_indices_handle_");

  const auto max_frames_in_flight{frame_state_->GetMaxFramesInFlight()};

  ssbo_proxy_ids_handle_ = Array<GlNativeStorageHandle>::WithCapacity(
      &general_allocator_, max_frames_in_flight);
  ssbo_indirect_proxies_handle_ = Array<GlNativeStorageHandle>::WithCapacity(
      &general_allocator_, max_frames_in_flight);
  ssbo_proxy_instances_handle_ = Array<GlNativeStorageHandle>::WithCapacity(
      &general_allocator_, max_frames_in_flight);

  ssbo_shadow_indirect_proxies_handle_ =
      Array<GlNativeStorageHandle>::WithCapacity(&general_allocator_,
                                                 max_frames_in_flight);
  ssbo_shadow_indirect_proxies_buffer_size_ =
      Array<GLsizei>::WithCapacity(&general_allocator_, max_frames_in_flight);

  ssbo_proxy_ids_buffer_size_ =
      Array<GLsizei>::WithCapacity(&general_allocator_, max_frames_in_flight);
  ssbo_indirect_proxies_buffer_size_ =
      Array<GLsizei>::WithCapacity(&general_allocator_, max_frames_in_flight);
  ssbo_proxy_instances_buffer_size_ =
      Array<GLsizei>::WithCapacity(&general_allocator_, max_frames_in_flight);

  ssbo_proxy_ids_handle_.Resize(max_frames_in_flight);
  ssbo_indirect_proxies_handle_.Resize(max_frames_in_flight);
  ssbo_proxy_instances_handle_.Resize(max_frames_in_flight);

  ssbo_proxy_ids_buffer_size_.Resize(max_frames_in_flight);
  ssbo_indirect_proxies_buffer_size_.Resize(max_frames_in_flight);
  ssbo_proxy_instances_buffer_size_.Resize(max_frames_in_flight);

  ssbo_shadow_indirect_proxies_handle_.Resize(max_frames_in_flight);
  ssbo_shadow_indirect_proxies_buffer_size_.Resize(max_frames_in_flight);

  for (FrameInFlightIndex i{0}; i < max_frames_in_flight; ++i) {
    ssbo_proxy_ids_handle_[i] = kInvalidGlNativeStorageHandle;
    ssbo_indirect_proxies_handle_[i] = kInvalidGlNativeStorageHandle;
    ssbo_proxy_instances_handle_[i] = kInvalidGlNativeStorageHandle;
    ssbo_shadow_indirect_proxies_handle_[i] = kInvalidGlNativeStorageHandle;

    ssbo_proxy_ids_buffer_size_[i] = 0;
    ssbo_indirect_proxies_buffer_size_[i] = 0;
    ssbo_proxy_instances_buffer_size_[i] = 0;
    ssbo_shadow_indirect_proxies_buffer_size_[i] = 0;

    glCreateBuffers(1, &ssbo_proxy_ids_handle_[i]);
    glCreateBuffers(1, &ssbo_indirect_proxies_handle_[i]);
    glCreateBuffers(1, &ssbo_proxy_instances_handle_[i]);
    glCreateBuffers(1, &ssbo_shadow_indirect_proxies_handle_[i]);

    COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_proxy_ids_handle_[i],
                                     "ssbo_proxy_ids_handle_");
    COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_indirect_proxies_handle_[i],
                                     "ssbo_indirect_proxies_handle_");
    COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_proxy_instances_handle_[i],
                                     "ssbo_proxy_instances_handle_");
    COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_shadow_indirect_proxies_handle_[i],
                                     "ssbo_shadow_indirect_proxies_handle_");
  }
}

void RenderProxyHandler::DestroyBuffers() {
  if (staging_ssbo_proxy_local_datas_handle_ != kInvalidGlNativeStorageHandle) {
    glDeleteBuffers(1, &staging_ssbo_proxy_local_datas_handle_);
    staging_ssbo_proxy_local_datas_handle_ = kInvalidGlNativeStorageHandle;
    staging_ssbo_proxy_local_datas_buffer_size_ = 0;
  }

  if (ssbo_proxy_local_datas_handle_ != kInvalidGlNativeStorageHandle) {
    glDeleteBuffers(1, &ssbo_proxy_local_datas_handle_);
    ssbo_proxy_local_datas_handle_ = kInvalidGlNativeStorageHandle;
    ssbo_proxy_local_datas_buffer_size_ = 0;
  }

  if (ssbo_matrix_palettes_handle_ != kInvalidGlNativeStorageHandle) {
    glDeleteBuffers(1, &ssbo_matrix_palettes_handle_);
    ssbo_matrix_palettes_handle_ = kInvalidGlNativeStorageHandle;
    ssbo_matrix_palettes_buffer_size_ = 0;
  }

  if (ssbo_word_indices_handle_ != kInvalidGlNativeStorageHandle) {
    glDeleteBuffers(1, &ssbo_word_indices_handle_);
    ssbo_word_indices_handle_ = kInvalidGlNativeStorageHandle;
    ssbo_word_indices_buffer_size_ = 0;
  }

  for (usize i{0}; i < ssbo_proxy_ids_handle_.GetSize(); ++i) {
    if (ssbo_proxy_ids_handle_[i] != kInvalidGlNativeStorageHandle) {
      glDeleteBuffers(1, &ssbo_proxy_ids_handle_[i]);
      ssbo_proxy_ids_handle_[i] = kInvalidGlNativeStorageHandle;
      ssbo_proxy_ids_buffer_size_[i] = 0;
    }

    if (ssbo_indirect_proxies_handle_[i] != kInvalidGlNativeStorageHandle) {
      glDeleteBuffers(1, &ssbo_indirect_proxies_handle_[i]);
      ssbo_indirect_proxies_handle_[i] = kInvalidGlNativeStorageHandle;
      ssbo_indirect_proxies_buffer_size_[i] = 0;
    }

    if (ssbo_shadow_indirect_proxies_handle_[i] !=
        kInvalidGlNativeStorageHandle) {
      glDeleteBuffers(1, &ssbo_shadow_indirect_proxies_handle_[i]);
      ssbo_shadow_indirect_proxies_handle_[i] = kInvalidGlNativeStorageHandle;
      ssbo_shadow_indirect_proxies_buffer_size_[i] = 0;
    }

    if (ssbo_proxy_instances_handle_[i] != kInvalidGlNativeStorageHandle) {
      glDeleteBuffers(1, &ssbo_proxy_instances_handle_[i]);
      ssbo_proxy_instances_handle_[i] = kInvalidGlNativeStorageHandle;
      ssbo_proxy_instances_buffer_size_[i] = 0;
    }
  }

  ssbo_indirect_proxies_handle_.Release();
  ssbo_proxy_instances_handle_.Release();
  ssbo_indirect_proxies_buffer_size_.Release();
  ssbo_shadow_indirect_proxies_handle_.Release();
  ssbo_shadow_indirect_proxies_buffer_size_.Release();
  ssbo_proxy_instances_buffer_size_.Release();
  ssbo_proxy_ids_handle_.Release();
  ssbo_proxy_ids_buffer_size_.Release();
}

#ifdef COMET_DEBUG_RENDERING
void RenderProxyHandler::InitializeDebugData() {
  for (u32 i{0}; i < kDebugDataBufferCount_; ++i) {
    glGenBuffers(1, &ssbo_debug_data_handle_[i]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_data_handle_[i]);

    const auto size{static_cast<GLsizei>(sizeof(GpuDebugData))};
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_READ);
    ssbo_debug_data_buffer_size_[i] = size;

    void* debug_data{glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size,
                                      GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)};
    COMET_ASSERT(debug_data != nullptr,
                 "RenderProxyHandler::InitializeDebugData",
                 "failed to map debug data buffer", "buffer_index", i);

    debug_data_[i] = static_cast<GpuDebugData*>(debug_data);
  }

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
}

void RenderProxyHandler::DestroyDebugData() {
  for (u32 i{0}; i < kDebugDataBufferCount_; ++i) {
    if (ssbo_debug_data_handle_[i] != kInvalidGlNativeStorageHandle) {
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_data_handle_[i]);
      glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
      glDeleteBuffers(1, &ssbo_debug_data_handle_[i]);
      ssbo_debug_data_handle_[i] = kInvalidGlNativeStorageHandle;
      ssbo_debug_data_buffer_size_[i] = 0;
      debug_data_[i] = nullptr;
    }
  }
}
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
void RenderProxyHandler::InitializeCullingDebug() {
  glGenBuffers(1, &ssbo_debug_aabbs_handle_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_aabbs_handle_);
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_debug_aabbs_handle_,
                                   "ssbo_debug_aabbs_handle_");
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               kDefaultProxyCount_ * sizeof(GpuDebugAabb), nullptr,
               GL_DYNAMIC_DRAW);
  ssbo_debug_aabbs_buffer_size_ =
      static_cast<GLsizei>(kDefaultProxyCount_ * sizeof(GpuDebugAabb));

  glGenBuffers(1, &ssbo_debug_lines_handle_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_lines_handle_);
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_debug_lines_handle_,
                                   "ssbo_debug_lines_handle_");
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               kDefaultProxyCount_ * 24 * sizeof(math::Vec4), nullptr,
               GL_DYNAMIC_DRAW);
  ssbo_debug_lines_buffer_size_ =
      static_cast<GLsizei>(kDefaultProxyCount_ * 24 * sizeof(math::Vec4));

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
}

void RenderProxyHandler::DestroyCullingDebug() {
  if (ssbo_debug_aabbs_handle_ != kInvalidGlNativeStorageHandle) {
    glDeleteBuffers(1, &ssbo_debug_aabbs_handle_);
    ssbo_debug_aabbs_handle_ = kInvalidGlNativeStorageHandle;
    ssbo_debug_aabbs_buffer_size_ = 0;
  }

  if (ssbo_debug_lines_handle_ != kInvalidGlNativeStorageHandle) {
    glDeleteBuffers(1, &ssbo_debug_lines_handle_);
    ssbo_debug_lines_handle_ = kInvalidGlNativeStorageHandle;
    ssbo_debug_lines_buffer_size_ = 0;
  }
}
#endif  // COMET_DEBUG_CULLING

void RenderProxyHandler::DestroyLiveProxyMaterials() {
  for (u32 i{0}; i < render_proxy_count_; ++i) {
    auto& proxy{proxies_[i]};

    if (proxy.material_handle) {
      material_handler_->Destroy(proxy.material_handle);
      proxy.material_handle.Invalidate();
    }
  }
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet