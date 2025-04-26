// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "opengl_render_proxy_handler.h"

#include <utility>

#include "comet/core/algorithm/back_insert_iterator.h"
#include "comet/core/algorithm/inplace_merge.h"
#include "comet/core/algorithm/set_difference.h"
#include "comet/core/algorithm/sort.h"
#include "comet/entity/entity_manager.h"
#include "comet/geometry/component/mesh_component.h"
#include "comet/geometry/geometry_common.h"
#include "comet/physics/component/transform_component.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/camera/camera_manager.h"
#include "comet/rendering/driver/opengl/data/opengl_material.h"
#include "comet/rendering/driver/opengl/data/opengl_mesh.h"
#include "comet/rendering/driver/opengl/opengl_debug.h"

namespace comet {
namespace rendering {
namespace gl {
void RenderProxyHandler::Initialize() {
  Handler::Initialize();
  proxy_local_data_allocator_.Initialize();
  general_allocator_.Initialize();
  proxy_local_datas_ = Array<GpuRenderProxyLocalData>{
      &proxy_local_data_allocator_, kDefaultProxyCount_};
  batch_entries_ =
      Array<RenderBatchEntry>{&general_allocator_, kDefaultProxyCount_};
  entity_id_to_proxy_id_map_ = Map<entity::EntityId, RenderProxyId>{
      &general_allocator_, kDefaultProxyCount_};
  proxy_id_to_entity_id_map_ =
      Array<entity::EntityId>{&general_allocator_, kDefaultProxyCount_};

  ShaderDescr shader_descr{};
  shader_descr.resource_path =
      COMET_TCHAR("shaders/opengl/sparse_upload.gl.cshader");
  sparse_upload_shader_ = shader_handler_->Generate(shader_descr);

#ifdef COMET_DEBUG_RENDERING
  InitializeDebugData();
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  InitializeCullingDebug();
#endif  // COMET_DEBUG_CULLING

  glGenBuffers(1, &ssbo_indirect_proxies_handle_);
  glGenBuffers(1, &ssbo_proxy_instances_handle_);
  glGenBuffers(1, &ssbo_proxy_local_datas_handle_);
  glGenBuffers(1, &ssbo_proxy_ids_handle_);
  glGenBuffers(1, &ssbo_word_indices_handle_);

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssbo_indirect_proxies_handle_);
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_indirect_proxies_handle_,
                                   "ssbo_indirect_proxies_handle_");
  glBindBuffer(GL_COPY_WRITE_BUFFER, ssbo_proxy_instances_handle_);
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_proxy_instances_handle_,
                                   "ssbo_proxy_instances_handle_");
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_proxy_local_datas_handle_);
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_proxy_local_datas_handle_,
                                   "ssbo_proxy_local_datas_handle_");
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_proxy_ids_handle_);
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_proxy_ids_handle_,
                                   "ssbo_proxy_ids_handle_");
  glBindBuffer(GL_COPY_WRITE_BUFFER, ssbo_word_indices_handle_);
  COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_word_indices_handle_,
                                   "ssbo_word_indices_handle_");
}

void RenderProxyHandler::Shutdown() {
#ifdef COMET_DEBUG_RENDERING
  DestroyDebugData();
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  DestroyCullingDebug();
#endif  // COMET_DEBUG_CULLING

  DestroyUpdateTemporaryStructures();

  proxy_local_datas_.Clear();
  batch_entries_.Clear();
  proxy_id_to_entity_id_map_ = {};
  entity_id_to_proxy_id_map_ = {};

  general_allocator_.Destroy();
  proxy_local_data_allocator_.Destroy();

  update_frame_ = kInvalidFrameCount;
  render_proxy_count_ = 0;
  render_proxy_visible_count_ = 0;

  if (ssbo_indirect_proxies_handle_ != kInvalidStorageHandle) {
    glDeleteBuffers(1, &ssbo_indirect_proxies_handle_);
    ssbo_indirect_proxies_buffer_size_ = 0;
  }

  if (ssbo_proxy_instances_handle_ != kInvalidStorageHandle) {
    glDeleteBuffers(1, &ssbo_proxy_instances_handle_);
    ssbo_proxy_instances_buffer_size_ = 0;
  }

  if (ssbo_proxy_local_datas_handle_ != kInvalidStorageHandle) {
    glDeleteBuffers(1, &ssbo_proxy_local_datas_handle_);
    ssbo_proxy_local_datas_buffer_size_ = 0;
  }

  if (ssbo_proxy_ids_handle_ != kInvalidStorageHandle) {
    glDeleteBuffers(1, &ssbo_proxy_ids_handle_);
    ssbo_proxy_ids_buffer_size_ = 0;
  }

  if (ssbo_word_indices_handle_ != kInvalidStorageHandle) {
    glDeleteBuffers(1, &ssbo_word_indices_handle_);
    ssbo_word_indices_buffer_size_ = 0;
  }

  if (sparse_upload_shader_ != nullptr) {
    shader_handler_->Destroy(sparse_upload_shader_);
    sparse_upload_shader_ = nullptr;
  }

  Handler::Shutdown();
}

void RenderProxyHandler::Update(frame::FramePacket* packet) {
  COMET_PROFILE("RenderProxyHandler::Update");
  auto frame_count{static_cast<FrameCount>(packet->frame_count)};

  if (update_frame_ == frame_count) {
    return;
  }

  GenerateUpdateTemporaryStructures(packet);
  ApplyRenderProxyChanges(packet);
  ProcessBatches();
  UploadRenderProxyLocalData();
  PrepareRenderProxyDrawData();
  CommitUpdate(packet);

  update_frame_ = frame_count;
}

void RenderProxyHandler::Cull(Shader* shader) {
  COMET_PROFILE("RenderProxyHandler::Cull");
  shader_handler_->Bind(shader, ShaderBindType::Compute);

#ifdef COMET_DEBUG_RENDERING
  render_proxy_visible_count_ = debug_data_->visible_count;
  debug_data_->visible_count = 0;
#endif  // COMET_DEBUG_RENDERING

  glDispatchCompute(static_cast<u32>((batch_entries_.GetSize() +
                                      rendering::kShaderLocalSize - 1) /
                                     rendering::kShaderLocalSize),
                    1, 1);
}

void RenderProxyHandler::Draw(Shader* shader, FrameCount frame_count) {
  COMET_PROFILE("RenderProxyHandler::Draw");

  if (batch_groups_->IsEmpty()) {
    return;
  }

  mesh_handler_->Bind();
  auto last_mat_id{kInvalidMaterialId};
  shader_handler_->Bind(shader, ShaderBindType::Graphics);

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssbo_indirect_proxies_handle_);

  for (const auto& group : *batch_groups_) {
    auto& instance{indirect_batches_->Get(group.offset)};
    const auto* proxy{instance.proxy};

    if (proxy->mat_id != last_mat_id) {
      shader_handler_->UpdateInstance(shader, frame_count, proxy->mat_id);
      shader_handler_->BindInstance(shader, proxy->mat_id);
      last_mat_id = proxy->mat_id;
    }

    glMultiDrawElementsIndirect(
        shader->topology, GL_UNSIGNED_INT,
        reinterpret_cast<const void*>(group.offset *
                                      sizeof(GpuIndirectRenderProxy)),
        group.count, sizeof(GpuIndirectRenderProxy));
  }

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, kInvalidStorageHandle);
}

#ifdef COMET_DEBUG_CULLING
void RenderProxyHandler::DebugCull(Shader* shader) {
  COMET_PROFILE("RenderProxyHandler::DebugCull");

  if (render_proxy_count_ == 0) {
    return;
  }

  auto ssbo_debug_lines_size{
      static_cast<GLsizei>(render_proxy_count_ * 24 * sizeof(math::Vec3))};

  if (ssbo_debug_lines_buffer_size_ < ssbo_debug_lines_size) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_lines_handle_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_debug_lines_size, nullptr,
                 GL_DYNAMIC_DRAW);
    ssbo_debug_lines_buffer_size_ = ssbo_debug_lines_size;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidStorageHandle);
  }

  ShaderStoragesUpdate update{};
  update.ssbo_debug_aabbs_handle = ssbo_debug_aabbs_handle_;
  update.ssbo_debug_lines_handle = ssbo_debug_lines_handle_;

  shader_handler_->UpdateStorages(shader, update);
  shader_handler_->Bind(shader, ShaderBindType::Compute);

  glDispatchCompute(
      static_cast<u32>((render_proxy_count_ + rendering::kShaderLocalSize - 1) /
                       rendering::kShaderLocalSize),
      1, 1);
}

void RenderProxyHandler::DrawDebugCull(Shader* shader) {
  COMET_PROFILE("RenderProxyHandler::DrawDebugCull");

  if (render_proxy_count_ == 0) {
    return;
  }

  shader_handler_->Bind(shader, ShaderBindType::Graphics);
  glBindBuffer(GL_ARRAY_BUFFER, ssbo_debug_lines_handle_);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(math::Vec4), nullptr);
  glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(render_proxy_count_ * 24));
  glDisableVertexAttribArray(kInvalidVertexAttributeHandle);
  glBindBuffer(GL_ARRAY_BUFFER, kInvalidStorageHandle);
}
#endif  // COMET_DEBUG_CULLING

u32 RenderProxyHandler::GetRenderProxyCount() const noexcept {
  return static_cast<u32>(render_proxy_count_);
}

u32 RenderProxyHandler::GetVisibleCount() const noexcept {
  return static_cast<u32>(render_proxy_visible_count_);
}

bool RenderProxyHandler::OnRenderBatchSort(const RenderBatchEntry& a,
                                           const RenderBatchEntry& b) {
  if (a.sort_key != b.sort_key) {
    return a.sort_key < b.sort_key;
  }

  return a.proxy->id < b.proxy->id;
}

void RenderProxyHandler::GenerateUpdateTemporaryStructures(
    const frame::FramePacket* packet) {
  pending_proxy_ids_ = COMET_FRAME_ORDERED_SET(
      RenderProxyId, packet->added_geometries->GetSize() +
                         packet->dirty_transforms->GetSize() +
                         packet->removed_geometries->GetSize());

  pending_proxy_local_data_ = COMET_FRAME_ARRAY(
      GpuRenderProxyLocalData, pending_proxy_ids_->GetCapacity());

  pending_proxy_indices_ = COMET_FRAME_ARRAY(usize);
  pending_proxy_indices_->Reserve(packet->added_geometries->GetSize() +
                                  packet->removed_geometries->GetSize());

  destroyed_proxies_ =
      COMET_FRAME_ARRAY(RenderProxy, packet->removed_geometries->GetSize());

  destroyed_batch_entries_ = COMET_FRAME_ARRAY(
      RenderBatchEntry, packet->removed_geometries->GetSize());

  indirect_batches_ =
      COMET_FRAME_ARRAY(RenderIndirectBatch, kDefaultRenderIndirectBatchCount_);

  batch_groups_ =
      COMET_FRAME_ARRAY(RenderBatchGroup, kDefaultRenderBatchGroupCount_);
}

void RenderProxyHandler::DestroyUpdateTemporaryStructures() {
  pending_proxy_ids_ = nullptr;
  pending_proxy_local_data_ = nullptr;
  pending_proxy_indices_ = nullptr;
  destroyed_proxies_ = nullptr;
  destroyed_batch_entries_ = nullptr;
  indirect_batches_ = nullptr;
  batch_groups_ = nullptr;
}

void RenderProxyHandler::ApplyRenderProxyChanges(
    const frame::FramePacket* packet) {
  COMET_PROFILE("RenderProxyHandler::ApplyRenderProxyChanges");

  auto generated_proxy_count{packet->added_geometries->GetSize()};
  auto destroyed_proxy_count{packet->removed_geometries->GetSize()};
  auto resize_delta{generated_proxy_count > destroyed_proxy_count
                        ? generated_proxy_count - destroyed_proxy_count
                        : 0};

  if (resize_delta > 0) {
    proxy_id_to_entity_id_map_.Resize(proxy_id_to_entity_id_map_.GetSize() +
                                      resize_delta);
  }

  DestroyRenderProxies(packet->removed_geometries);
  GenerateRenderProxies(packet->added_geometries);
  UpdateRenderProxies(packet->dirty_meshes, packet->dirty_transforms);
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

  auto generated_proxy_count{geometries->GetSize()};

  if (generated_proxy_count == 0) {
    return;
  }

  for (usize i{0}; i < generated_proxy_count; ++i) {
    COMET_ASSERT(render_proxy_count_ != kMaxRenderProxyCount_,
                 "Max count of render proxies reached!");
    const auto& geometry{geometries->Get(i)};

    auto& local_data{proxy_local_datas_.EmplaceBack()};
    local_data.local_center = math::Vec4{geometry.local_center, .0f};
    local_data.local_max_extents = math::Vec4{geometry.local_max_extents, .0f};
    local_data.transform = geometry.transform;

    auto& new_proxy{proxies_[render_proxy_count_++]};
    new_proxy.id = static_cast<RenderProxyId>(render_proxy_count_ - 1);
    auto* material{material_handler_->TryGet(geometry.material->id)};

    if (material == nullptr) {
      material = material_handler_->Generate(geometry.material);
      shader_handler_->BindMaterial(material);
    }

    new_proxy.mat_id = material->id;
    new_proxy.mesh_handle = mesh_handler_->GetHandle(geometry.mesh_id);

    COMET_ASSERT(new_proxy.mesh_handle != kInvalidMeshProxyHandle,
                 "Invalid mesh proxy handle retrieved!");

    entity_id_to_proxy_id_map_[geometry.entity_id] = new_proxy.id;
    proxy_id_to_entity_id_map_[new_proxy.id] = geometry.entity_id;
    pending_proxy_ids_->Add(new_proxy.id);
    pending_proxy_indices_->PushBack(new_proxy.id);
    pending_proxy_local_data_->PushBack(local_data);
  }
}

void RenderProxyHandler::UpdateRenderProxies(
    const frame::DirtyMeshes* meshes,
    const frame::DirtyTransforms* transforms) {
  COMET_PROFILE("RenderProxyHandler::UpdateRenderProxies");

  auto updated_mesh_count{meshes->GetSize()};
  auto updated_transform_count{transforms->GetSize()};

  if (updated_mesh_count == 0 && updated_transform_count == 0) {
    return;
  }

  for (usize i{0}; i < updated_mesh_count; ++i) {
    auto& updated_mesh{meshes->Get(i)};

    COMET_ASSERT(entity_id_to_proxy_id_map_.IsContained(updated_mesh.entity_id),
                 "Tried to update non-existing mesh with entity #",
                 updated_mesh.entity_id, "!");

    auto proxy_id{entity_id_to_proxy_id_map_[updated_mesh.entity_id]};

    auto& updated_proxy{proxies_[proxy_id]};
    pending_proxy_ids_->Add(updated_proxy.id);

    auto& local_data{proxy_local_datas_[proxy_id]};
    local_data.local_center = math::Vec4{updated_mesh.local_center, .0f};
    local_data.local_max_extents =
        math::Vec4{updated_mesh.local_max_extents, .0f};

    pending_proxy_local_data_->PushBack(local_data);
  }

  for (usize i{0}; i < updated_transform_count; ++i) {
    auto& updated_transform{transforms->Get(i)};

    COMET_ASSERT(
        entity_id_to_proxy_id_map_.IsContained(updated_transform.entity_id),
        "Tried to update non-existing transform with entity #",
        updated_transform.entity_id, "!");

    auto proxy_id{entity_id_to_proxy_id_map_[updated_transform.entity_id]};

    auto& updated_proxy{proxies_[proxy_id]};
    pending_proxy_ids_->Add(updated_proxy.id);

    auto& local_data{proxy_local_datas_[proxy_id]};
    local_data.transform = updated_transform.transform;

    pending_proxy_local_data_->PushBack(local_data);
  }
}

void RenderProxyHandler::DestroyRenderProxies(
    const frame::RemovedGeometries* geometries) {
  COMET_PROFILE("RenderProxyHandler::DestroyRenderProxies");

  if (geometries->IsEmpty()) {
    return;
  }

  for (const auto& geometry : *geometries) {
    auto proxy_id_ptr{entity_id_to_proxy_id_map_.TryGet(geometry.entity_id)};

    if (proxy_id_ptr == nullptr) {
      COMET_LOG_RENDERING_WARNING("Render proxy with entity #",
                                  geometry.entity_id,
                                  " not found! Ignoring destruction...");
      continue;
    }

    auto proxy_id{*proxy_id_ptr};
    COMET_ASSERT(proxy_id < render_proxy_count_,
                 "Invalid render proxy ID: ", proxy_id, " > ",
                 render_proxy_count_, "!");

    // Preserve a valid reference to the proxy before it gets overwritten during
    // the swap.
    auto& destroyed_proxy{destroyed_proxies_->EmplaceBack(proxies_[proxy_id])};

    RenderBatchEntry destroyed_batch_entry{};
    destroyed_batch_entry.proxy = &destroyed_proxy;
    destroyed_batch_entry.sort_key =
        GenerateRenderProxySortKey(*destroyed_batch_entry.proxy);
    destroyed_batch_entries_->PushBack(destroyed_batch_entry);

    auto old_proxy_id{static_cast<RenderProxyId>(render_proxy_count_ - 1)};

    // Swap destroyed proxy with last active proxy for contiguous storage.
    if (proxy_id != old_proxy_id) {
      proxies_[proxy_id] = proxies_[old_proxy_id];
      proxy_local_datas_[proxy_id] = proxy_local_datas_[old_proxy_id];
      entity_id_to_proxy_id_map_[proxy_id_to_entity_id_map_[old_proxy_id]] =
          proxy_id;

      pending_proxy_ids_->Add(proxy_id);
      pending_proxy_local_data_->PushBack(proxy_local_datas_[proxy_id]);
    }

    entity_id_to_proxy_id_map_.Remove(geometry.entity_id);
    --render_proxy_count_;
  }

  if (destroyed_batch_entries_->IsEmpty()) {
    return;
  }

  // Sort the destroyed batch entries to align with batch_entries_ for
  // set_difference.
  // batch_entries_ is already sorted, so no need to do it here.
  Sort(destroyed_batch_entries_->begin(), destroyed_batch_entries_->end(),
       OnRenderBatchSort);

  Array<RenderBatchEntry> filtered_batches{&general_allocator_,
                                           batch_entries_.GetSize()};

  SetDifference(batch_entries_.begin(), batch_entries_.end(),
                destroyed_batch_entries_->begin(),
                destroyed_batch_entries_->end(), BackInserter(filtered_batches),
                OnRenderBatchSort);

  batch_entries_ = std::move(filtered_batches);
}

void RenderProxyHandler::GenerateBatchEntries() {
  COMET_PROFILE("RenderProxyHandler::GenerateBatchEntries");

  if (pending_proxy_indices_ == nullptr) {
    return;
  }

  auto generated_proxy_count{pending_proxy_indices_->GetSize()};

  if (generated_proxy_count == 0) {
    return;
  }

  new_batch_entries_ =
      Array<RenderBatchEntry>{&general_allocator_, generated_proxy_count};

  for (auto i : *pending_proxy_indices_) {
    auto& new_proxy{proxies_[i]};
    auto& batch{new_batch_entries_.EmplaceBack()};
    batch.sort_key = GenerateRenderProxySortKey(new_proxy);
    batch.proxy = &new_proxy;
  }

  Sort(new_batch_entries_.begin(), new_batch_entries_.end(), OnRenderBatchSort);

  auto batch_count{batch_entries_.GetSize()};
  auto new_batch_count{new_batch_entries_.GetSize()};

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
  auto* current_indirect_batch{&indirect_batches_->EmplaceBack()};
  current_indirect_batch->offset = 0;
  current_indirect_batch->count = 1;
  current_indirect_batch->proxy = first_batch->proxy;

  usize last_batch_index{0};

  for (usize batch_id{1}; batch_id < batch_entries_.GetSize(); ++batch_id) {
    auto& batch{batch_entries_[batch_id]};
    auto* proxy{batch.proxy};
    auto& last_batch{indirect_batches_->Get(last_batch_index)};

    auto is_same_mesh{proxy->mesh_handle == last_batch.proxy->mesh_handle};
    auto is_same_material{proxy->mat_id == last_batch.proxy->mat_id};

    if (is_same_mesh && is_same_material) {
      ++last_batch.count;
      continue;
    }

    current_indirect_batch = &indirect_batches_->EmplaceBack();
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

  auto* current_group{&batch_groups_->EmplaceBack()};
  current_group->offset = 0;
  current_group->count = 1;

  for (usize i{1}; i < indirect_batches_->GetSize(); ++i) {
    auto& anchor_batch{indirect_batches_->Get(current_group->offset)};
    auto& batch{indirect_batches_->Get(i)};

    if (anchor_batch.proxy->mat_id == batch.proxy->mat_id) {
      ++current_group->count;
      continue;
    }

    current_group = &batch_groups_->EmplaceBack();
    current_group->offset = static_cast<u32>(i);
    current_group->count = 1;
  }
}

void RenderProxyHandler::UploadRenderProxyLocalData() {
  COMET_PROFILE("RenderProxyHandler::UploadRenderProxyLocalData");

  if (pending_proxy_local_data_->IsEmpty()) {
    return;
  }

#ifdef COMET_DEBUG_CULLING
  auto ssbo_debug_aabbs_buffer_size{
      static_cast<GLsizei>(render_proxy_count_ * sizeof(GpuDebugAabb))};

  if (ssbo_debug_aabbs_buffer_size > ssbo_debug_aabbs_buffer_size_) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_aabbs_handle_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_debug_aabbs_buffer_size,
                 nullptr, GL_DYNAMIC_DRAW);
    ssbo_debug_aabbs_buffer_size_ = ssbo_debug_aabbs_buffer_size;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidStorageHandle);
  }
#endif  // COMET_DEBUG_CULLING

  auto ssbo_proxy_ids_buffer_size{static_cast<GLsizei>(
      proxy_local_datas_.GetSize() * sizeof(RenderProxyId))};

  if (ssbo_proxy_ids_buffer_size > ssbo_proxy_ids_buffer_size_) {
    glBindBuffer(GL_COPY_WRITE_BUFFER, ssbo_proxy_ids_handle_);
    glBufferData(GL_COPY_WRITE_BUFFER, ssbo_proxy_ids_buffer_size, nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_COPY_WRITE_BUFFER, kInvalidStorageHandle);
    ssbo_proxy_ids_buffer_size_ = ssbo_proxy_ids_buffer_size;
  }

  if (pending_proxy_local_data_->GetSize() >
      proxy_local_datas_.GetSize() * kReuploadAllLocalDataThreshold_) {
    UploadAllRenderProxyLocalData();
  } else {
    UploadPendingRenderProxyLocalData();
  }

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidStorageHandle);
}

void RenderProxyHandler::UploadAllRenderProxyLocalData() {
  COMET_PROFILE("RenderProxyHandler::UploadAllRenderProxyLocalData");

  auto ssbo_proxy_local_datas_buffer_size{static_cast<GLsizei>(
      proxy_local_datas_.GetSize() * sizeof(GpuRenderProxyLocalData))};

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_proxy_local_datas_handle_);

  if (ssbo_proxy_local_datas_buffer_size >
      ssbo_proxy_local_datas_buffer_size_) {
    glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_proxy_local_datas_buffer_size,
                 nullptr, GL_DYNAMIC_DRAW);
    ssbo_proxy_local_datas_buffer_size_ = ssbo_proxy_local_datas_buffer_size;
  } else {
    glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_proxy_local_datas_buffer_size,
                 nullptr, GL_DYNAMIC_DRAW);
  }

  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                  ssbo_proxy_local_datas_buffer_size,
                  proxy_local_datas_.GetData());
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidStorageHandle);
}

void RenderProxyHandler::PrepareRenderProxyDrawData() {
  COMET_PROFILE("RenderProxyHandler::PrepareRenderProxyDrawData");
  ReallocateRenderProxyDrawBuffers();
  PopulateRenderProxyDrawData();
}

void RenderProxyHandler::UploadPendingRenderProxyLocalData() {
  COMET_PROFILE("RenderProxyHandler::UploadPendingRenderProxyLocalData");
  auto pending_count{pending_proxy_ids_->GetSize()};
  auto ssbo_proxy_local_datas_buffer_size{
      static_cast<GLsizei>(pending_count * sizeof(GpuRenderProxyLocalData))};

  glBindBuffer(GL_COPY_WRITE_BUFFER, ssbo_proxy_local_datas_handle_);

  if (ssbo_proxy_local_datas_buffer_size >
      ssbo_proxy_local_datas_buffer_size_) {
    glBufferData(GL_COPY_WRITE_BUFFER, ssbo_proxy_local_datas_buffer_size,
                 pending_proxy_local_data_->GetData(), GL_DYNAMIC_DRAW);
    ssbo_proxy_local_datas_buffer_size_ = ssbo_proxy_local_datas_buffer_size;
  } else {
    glBufferSubData(GL_COPY_WRITE_BUFFER, 0, ssbo_proxy_local_datas_buffer_size,
                    pending_proxy_local_data_->GetData());
  }

  auto word_count_per_data{sizeof(GpuRenderProxyLocalData) /
                           sizeof(ShaderWord)};

  auto total_word_count{static_cast<u32>(pending_count * word_count_per_data)};
  auto ssbo_word_indices_buffer_size{
      static_cast<GLsizei>(total_word_count * sizeof(ShaderWord))};

  glBindBuffer(GL_COPY_WRITE_BUFFER, ssbo_word_indices_handle_);

  if (ssbo_word_indices_buffer_size > ssbo_word_indices_buffer_size_) {
    glBufferData(GL_COPY_WRITE_BUFFER, ssbo_word_indices_buffer_size, nullptr,
                 GL_DYNAMIC_DRAW);
    ssbo_word_indices_buffer_size_ = ssbo_word_indices_buffer_size;
  }

  auto* word_indices{
      static_cast<u32*>(glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY))};

  COMET_ASSERT(word_indices != nullptr,
               "Failed to map ssbo_word_indices_handle_!");
  u32 global_word_index{0};

  for (usize i{0}; i < pending_count; ++i) {
    auto proxy_word_offset{static_cast<ShaderWord>(word_count_per_data *
                                                   pending_proxy_ids_->Get(i))};

    for (u32 word_index{0}; word_index < word_count_per_data; ++word_index) {
      word_indices[global_word_index] = proxy_word_offset + word_index;
      ++global_word_index;
    }
  }

  glUnmapBuffer(GL_COPY_WRITE_BUFFER);

  ShaderStoragesUpdate update{};
  update.ssbo_word_indices_handle = ssbo_word_indices_handle_;
  update.ssbo_source_words_handle = ssbo_proxy_local_datas_handle_;

  shader_handler_->Bind(sparse_upload_shader_, ShaderBindType::Compute);
  shader_handler_->UpdateConstants(sparse_upload_shader_,
                                   {nullptr, &total_word_count});
  shader_handler_->UpdateStorages(sparse_upload_shader_, update);

  glDispatchCompute(
      static_cast<u32>((total_word_count + rendering::kShaderLocalSize - 1) /
                       rendering::kShaderLocalSize),
      1, 1);
}

void RenderProxyHandler::CommitUpdate(frame::FramePacket* packet) {
  COMET_PROFILE("RenderProxyHandler::CommitUpdate");
  packet->rendering_data = &storages_update_;

  storages_update_.ssbo_proxy_local_datas_handle =
      ssbo_proxy_local_datas_handle_;

  storages_update_.ssbo_proxy_ids_handle = ssbo_proxy_ids_handle_;
  storages_update_.ssbo_proxy_instances_handle = ssbo_proxy_instances_handle_;
  storages_update_.ssbo_indirect_proxies_handle = ssbo_indirect_proxies_handle_;
  storages_update_.ssbo_source_words_handle = kInvalidStorageBufferHandle;

#ifdef COMET_DEBUG_RENDERING
  storages_update_.ssbo_debug_data_handle = ssbo_debug_data_handle_;
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  storages_update_.ssbo_debug_aabbs_handle = ssbo_debug_aabbs_handle_;
  storages_update_.ssbo_debug_lines_handle = kInvalidStorageHandle;
#endif  // COMET_DEBUG_CULLING
}

void RenderProxyHandler::ReallocateRenderProxyDrawBuffers() {
  COMET_PROFILE("RenderProxyHandler::ReallocateRenderProxyDrawBuffers");

  if (indirect_batches_->IsEmpty()) {
    return;
  }

  auto indirect_proxy_size{static_cast<GLsizei>(
      indirect_batches_->GetSize() * sizeof(GpuIndirectRenderProxy))};
  auto proxy_instance_size{static_cast<GLsizei>(
      batch_entries_.GetSize() * sizeof(GpuRenderProxyInstance))};

  if (indirect_proxy_size > ssbo_indirect_proxies_buffer_size_) {
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssbo_indirect_proxies_handle_);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, indirect_proxy_size, nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, kInvalidStorageHandle);
    ssbo_indirect_proxies_buffer_size_ = indirect_proxy_size;
  }

  if (proxy_instance_size > ssbo_proxy_instances_buffer_size_) {
    glBindBuffer(GL_COPY_WRITE_BUFFER, ssbo_proxy_instances_handle_);
    glBufferData(GL_COPY_WRITE_BUFFER, proxy_instance_size, nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_COPY_WRITE_BUFFER, kInvalidStorageHandle);
    ssbo_proxy_instances_buffer_size_ = proxy_instance_size;
  }
}

void RenderProxyHandler::PopulateRenderProxyDrawData() {
  COMET_PROFILE("RenderProxyHandler::PopulateRenderProxyDrawData");

  if (indirect_batches_->IsEmpty()) {
    return;
  }

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssbo_indirect_proxies_handle_);

  auto* indirect_proxies_memory{static_cast<GpuIndirectRenderProxy*>(
      glMapBuffer(GL_DRAW_INDIRECT_BUFFER, GL_WRITE_ONLY))};

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_proxy_instances_handle_);

  auto* proxy_instances_memory{static_cast<GpuRenderProxyInstance*>(
      glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY))};

  usize proxy_instance_index{0};

  for (usize batch_id{0}; batch_id < indirect_batches_->GetSize(); ++batch_id) {
    PopulateRenderIndirectProxy(static_cast<BatchId>(batch_id),
                                indirect_proxies_memory);
    PopulateProxyInstances(static_cast<BatchId>(batch_id),
                           proxy_instances_memory, proxy_instance_index);
  }

  glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, kInvalidStorageHandle);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidStorageHandle);
}

void RenderProxyHandler::PopulateRenderIndirectProxy(
    BatchId batch_id, GpuIndirectRenderProxy* memory) {
  COMET_PROFILE("RenderProxyHandler::PopulateRenderIndirectProxy");
  auto& batch{indirect_batches_->Get(batch_id)};
  const auto* mesh_proxy{mesh_handler_->Get(batch.proxy->mesh_handle)};

  auto& indirect_proxy{memory[batch_id]};
  indirect_proxy.command.firstInstance = batch.offset;
  indirect_proxy.command.instanceCount = 0;
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

u64 RenderProxyHandler::GenerateRenderProxySortKey(const RenderProxy& proxy) {
  auto shader_hash{GenerateHash(proxy.mat_id)};
  auto mesh_material_hash{
      HashCombine(GenerateHash(proxy.mat_id), GenerateHash(proxy.mesh_handle))};
  return static_cast<u64>(shader_hash) << 32 | mesh_material_hash;
}

#ifdef COMET_DEBUG_RENDERING
void RenderProxyHandler::InitializeDebugData() {
  if (ssbo_debug_data_handle_ == kInvalidStorageHandle) {
    glGenBuffers(1, &ssbo_debug_data_handle_);
  }

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_data_handle_);
  auto size{static_cast<GLsizei>(sizeof(GpuDebugData))};

  glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_READ);
  ssbo_debug_data_buffer_size_ = size;

  void* debug_data{glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size,
                                    GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)};

  COMET_ASSERT(debug_data != nullptr, "Failed to map ssbo_debug_data_handle_!");

  debug_data_ = static_cast<GpuDebugData*>(debug_data);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void RenderProxyHandler::DestroyDebugData() {
  if (ssbo_debug_data_handle_ != kInvalidStorageHandle) {
    glDeleteBuffers(1, &ssbo_debug_data_handle_);
    ssbo_debug_data_buffer_size_ = 0;
  }
}
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
void RenderProxyHandler::InitializeCullingDebug() {
  if (ssbo_debug_aabbs_handle_ == kInvalidStorageHandle) {
    glGenBuffers(1, &ssbo_debug_aabbs_handle_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_aabbs_handle_);
    COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_debug_aabbs_handle_,
                                     "ssbo_debug_aabbs_handle_");
  } else {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_aabbs_handle_);
  }

  glBufferData(GL_SHADER_STORAGE_BUFFER,
               kDefaultProxyCount_ * sizeof(GpuDebugAabb), nullptr,
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidStorageHandle);

  if (ssbo_debug_lines_handle_ == kInvalidStorageHandle) {
    glGenBuffers(1, &ssbo_debug_lines_handle_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_lines_handle_);
    COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_debug_lines_handle_,
                                     "ssbo_debug_lines_handle_");
  } else {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_debug_lines_handle_);
  }

  glBufferData(GL_SHADER_STORAGE_BUFFER,
               kDefaultProxyCount_ * 24 * sizeof(math::Vec3), nullptr,
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidStorageHandle);
}

void RenderProxyHandler::DestroyCullingDebug() {
  if (ssbo_debug_aabbs_handle_ != kInvalidStorageHandle) {
    glDeleteBuffers(1, &ssbo_debug_aabbs_handle_);
    ssbo_debug_aabbs_handle_ = kInvalidStorageHandle;
  }

  if (ssbo_debug_lines_handle_ != kInvalidStorageHandle) {
    glDeleteBuffers(1, &ssbo_debug_lines_handle_);
    ssbo_debug_lines_handle_ = kInvalidStorageHandle;
  }
}
#endif  // COMET_DEBUG_CULLING
}  // namespace gl
}  // namespace rendering
}  // namespace comet
