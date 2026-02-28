// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_render_proxy_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/algorithm/back_insert_iterator.h"
#include "comet/core/algorithm/inplace_merge.h"
#include "comet/core/algorithm/set_difference.h"
#include "comet/core/algorithm/sort.h"
#include "comet/core/type/ordered_set.h"
#include "comet/entity/entity_id.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/data/vulkan_mesh.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
RenderProxyHandler::RenderProxyHandler(const RenderProxyHandlerDescr& descr)
    : Handler{descr},
      material_handler_{descr.material_handler},
      mesh_handler_{descr.mesh_handler},
      shader_handler_{descr.shader_handler} {
  COMET_ASSERT(material_handler_ != nullptr, "Material handler is null!");
  COMET_ASSERT(mesh_handler_ != nullptr, "Mesh handler is null!");
  COMET_ASSERT(shader_handler_ != nullptr, "Shader handler is null!");
}

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
  model_to_proxies_map_ = Map<entity::EntityId, RenderProxyModelBindings>{
      &general_allocator_, kDefaultProxyCount_};
  proxy_id_to_entity_id_map_ =
      Array<entity::EntityId>{&general_allocator_, kDefaultProxyCount_};

  ShaderDescr shader_descr{};
  shader_descr.resource_path =
      COMET_TCHAR("shaders/vulkan/sparse_upload.vk.cshader");
  sparse_upload_shader_ = shader_handler_->Generate(shader_descr);

  InitializeBuffers();

#ifdef COMET_DEBUG_RENDERING
  InitializeDebugData();
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  InitializeCullingDebug();
#endif  // COMET_DEBUG_CULLING
}

void RenderProxyHandler::Shutdown() {
#ifdef COMET_DEBUG_RENDERING
  DestroyDebugData();
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  DestroyCullingDebug();
#endif  // COMET_DEBUG_CULLING
  DestroyUpdateTemporaryStructures();

  proxy_local_datas_.Destroy();
  batch_entries_.Destroy();
  proxy_id_to_entity_id_map_.Destroy();
  model_to_proxies_map_.Destroy();
  entity_id_to_proxy_id_map_.Destroy();

  general_allocator_.Destroy();
  proxy_local_data_allocator_.Destroy();

  update_frame_ = kInvalidFrameIndex;
  render_proxy_count_ = 0;
  render_proxy_visible_count_ = 0;

  DestroyBuffers();

  if (sparse_upload_shader_ != nullptr) {
    shader_handler_->Destroy(sparse_upload_shader_);
  }

  Handler::Shutdown();
}

void RenderProxyHandler::Update(frame::FramePacket* packet) {
  COMET_PROFILE("RenderProxyHandler::Update");
  auto frame_count{context_->GetFrameCount()};

  if (update_frame_ == frame_count) {
    return;
  }

  GenerateUpdateTemporaryStructures(packet);
  ApplyRenderProxyChanges(packet);
  ProcessBatches();
  UploadRenderProxyLocalData();
  PrepareRenderProxyDrawData();
  CommitUpdate(packet);

  ApplyBufferMemoryBarriers(&post_update_barriers_,
                            context_->GetFrameData().command_buffer_handle,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                                VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);

  update_frame_ = frame_count;
}

void RenderProxyHandler::Reset() { DestroyUpdateTemporaryStructures(); }

void RenderProxyHandler::Cull(Shader* shader) {
  COMET_PROFILE("RenderProxyHandler::Cull");
  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  shader_handler_->Bind(shader, PipelineBindType::Compute);

#ifdef COMET_DEBUG_RENDERING
  auto& debug_data{debug_data_[GetDebugDataWriteIndex()]};
  render_proxy_visible_count_ = debug_data->visible_count;
  debug_data->visible_count = 0;
#endif  // COMET_DEBUG_RENDERING

  vkCmdDispatch(command_buffer_handle,
                static_cast<u32>((batch_entries_.GetSize() +
                                  rendering::kShaderLocalSize - 1) /
                                 rendering::kShaderLocalSize),
                1, 1);

  AddBufferMemoryBarrier(ssbo_proxy_ids_, cull_barriers_,
                         VK_ACCESS_SHADER_WRITE_BIT,
                         VK_ACCESS_INDIRECT_COMMAND_READ_BIT);

  AddBufferMemoryBarrier(ssbo_indirect_proxies_, cull_barriers_,
                         VK_ACCESS_SHADER_WRITE_BIT,
                         VK_ACCESS_INDIRECT_COMMAND_READ_BIT);

  ApplyBufferMemoryBarriers(&cull_barriers_, command_buffer_handle,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                            VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);

#ifdef COMET_DEBUG_RENDERING
  auto& ssbo_debug_data{ssbo_debug_data_[GetDebugDataWriteIndex()]};
  AddBufferMemoryBarrier(ssbo_debug_data, debug_data_barriers_,
                         VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT);

  ApplyBufferMemoryBarriers(&debug_data_barriers_, command_buffer_handle,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                            VK_PIPELINE_STAGE_HOST_BIT);
#endif  // COMET_DEBUG_RENDERING
}

void RenderProxyHandler::Draw(Shader* shader) {
  COMET_PROFILE("RenderProxyHandler::Draw");

  if (batch_groups_->IsEmpty()) {
    return;
  }

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  mesh_handler_->Bind();
  auto last_mat_id{kInvalidMaterialId};
  shader_handler_->Bind(shader, PipelineBindType::Graphics);

  for (const auto& group : *batch_groups_) {
    auto& instance{indirect_batches_->Get(group.offset)};
    const auto* proxy{instance.proxy};

    if (proxy->mat_id != last_mat_id) {
      shader_handler_->UpdateInstance(shader, proxy->mat_id);
      shader_handler_->BindInstance(shader, proxy->mat_id,
                                    PipelineBindType::Graphics);
      last_mat_id = proxy->mat_id;
    }

    vkCmdDrawIndexedIndirect(command_buffer_handle,
                             ssbo_indirect_proxies_.handle,
                             group.offset * sizeof(GpuIndirectRenderProxy),
                             group.count, sizeof(GpuIndirectRenderProxy));
  }
}

#ifdef COMET_DEBUG_CULLING
void RenderProxyHandler::DebugCull(Shader* shader) {
  COMET_PROFILE("RenderProxyHandler::DebugCull");

  if (render_proxy_count_ == 0) {
    return;
  }

  auto* allocator_handle{context_->GetAllocatorHandle()};

  ReallocateBuffer(
      ssbo_debug_lines_, allocator_handle,
      render_proxy_count_ * 24 * sizeof(math::Vec3),
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_debug_lines_");

  ShaderStoragesUpdate update{};
  update.ssbo_debug_aabbs_handle = ssbo_debug_aabbs_.handle;
  update.ssbo_debug_aabbs_size = ssbo_debug_aabbs_.size;

  update.ssbo_debug_lines_handle = ssbo_debug_lines_.handle;
  update.ssbo_debug_lines_size = ssbo_debug_lines_.size;

  shader_handler_->UpdateStorages(shader, update);

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  shader_handler_->Bind(shader, PipelineBindType::Compute);

  vkCmdDispatch(
      command_buffer_handle,
      static_cast<u32>(render_proxy_count_ + rendering::kShaderLocalSize - 1) /
          rendering::kShaderLocalSize,
      1, 1);
}

void RenderProxyHandler::DrawDebugCull(Shader* shader) {
  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  shader_handler_->Bind(shader, PipelineBindType::Graphics);

  VkBuffer vertex_buffers[] = {ssbo_debug_lines_.handle};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(command_buffer_handle, 0, 1, vertex_buffers, offsets);
  vkCmdDraw(command_buffer_handle, 24, 1, 0, 0);
  vkCmdDraw(command_buffer_handle, static_cast<u32>(render_proxy_count_ * 24),
            1, 0, 0);
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
  post_update_barriers_ =
      COMET_FRAME_ARRAY(VkBufferMemoryBarrier, kDefaultUpdateBarrierCapacity_);

  cull_barriers_ =
      COMET_FRAME_ARRAY(VkBufferMemoryBarrier, kDefaultCullBarrierCapacity_);

#ifdef COMET_DEBUG_RENDERING
  debug_data_barriers_ = COMET_FRAME_ARRAY(VkBufferMemoryBarrier,
                                           kDefaultDebugDataBarrierCapacity_);
#endif  // COMET_DEBUG_RENDERING

  shader_to_transfer_barriers_ = COMET_FRAME_ARRAY(
      VkBufferMemoryBarrier, kDefaultShaderToTransferBarrierCapacity_);

  // Rough estimate: if reallocations become frequent in a single frame, we may
  // need a smarter sizing strategy.
  pending_proxy_ids_ = COMET_FRAME_ORDERED_SET(
      RenderProxyId, packet->added_geometries->GetSize() + kDefaultProxyCount_);

  pending_proxy_local_data_ = COMET_FRAME_ARRAY(
      GpuRenderProxyLocalData, pending_proxy_ids_->GetCapacity());

  pending_proxy_indices_ = COMET_FRAME_ARRAY(usize);
  pending_proxy_indices_->Reserve(packet->added_geometries->GetSize() +
                                  packet->removed_geometries->GetSize());

  destroyed_proxies_ =
      COMET_FRAME_ARRAY(RenderProxy, packet->removed_geometries->GetSize());

  destroyed_batch_entries_ = COMET_FRAME_ARRAY(
      RenderBatchEntry, packet->removed_geometries->GetSize());

  destroyed_entity_ids_ = COMET_FRAME_ORDERED_SET(
      entity::EntityId, packet->removed_geometries->GetSize());

  indirect_batches_ =
      COMET_FRAME_ARRAY(RenderIndirectBatch, kDefaultRenderIndirectBatchCount_);

  batch_groups_ =
      COMET_FRAME_ARRAY(RenderBatchGroup, kDefaultRenderBatchGroupCount_);
}

void RenderProxyHandler::DestroyUpdateTemporaryStructures() {
  post_update_barriers_ = nullptr;
  cull_barriers_ = nullptr;
#ifdef COMET_DEBUG_RENDERING
  debug_data_barriers_ = nullptr;
#endif  // COMET_DEBUG_RENDERING
  shader_to_transfer_barriers_ = nullptr;
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
    auto* material{material_handler_->TryGet(geometry.material_resource->id)};

    if (material == nullptr) {
      material = material_handler_->Generate(geometry.material_resource);
      shader_handler_->BindMaterial(material);
    }

    new_proxy.mat_id = material->id;
    new_proxy.mesh_handle = mesh_handler_->GetHandle(geometry.mesh_id);

    COMET_ASSERT(new_proxy.mesh_handle != kInvalidMeshProxyHandle,
                 "Invalid mesh proxy handle retrieved!");

    entity_id_to_proxy_id_map_[geometry.entity_id] = new_proxy.id;
    proxy_id_to_entity_id_map_[new_proxy.id] = geometry.entity_id;

    new_proxy.model_entity_id = geometry.model_entity_id;
    RegisterModelProxy(geometry.model_entity_id, new_proxy.id);
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

    // Case: the mesh was destroyed during the same frame.
    if (destroyed_entity_ids_->IsContained(updated_mesh.entity_id)) {
      continue;
    }

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

    // Case: the mesh was transform during the same frame.
    if (destroyed_entity_ids_->IsContained(updated_transform.entity_id)) {
      continue;
    }

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
    destroyed_entity_ids_->Add(geometry.entity_id);
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

    // Preserve a valid reference to the proxy before it gets overwritten
    // during the swap.
    auto& destroyed_proxy{destroyed_proxies_->EmplaceBack(proxies_[proxy_id])};

    RenderBatchEntry destroyed_batch_entry{};
    destroyed_batch_entry.proxy = &destroyed_proxy;
    destroyed_batch_entry.sort_key =
        GenerateRenderProxySortKey(*destroyed_batch_entry.proxy);
    destroyed_batch_entries_->PushBack(destroyed_batch_entry);
    UnregisterModelProxy(geometry.model_entity_id, proxy_id);

    auto old_proxy_id{static_cast<RenderProxyId>(render_proxy_count_ - 1)};

    // Swap destroyed proxy with last active proxy for contiguous storage.
    if (proxy_id != old_proxy_id) {
      proxies_[proxy_id] = proxies_[old_proxy_id];
      proxy_local_datas_[proxy_id] = proxy_local_datas_[old_proxy_id];
      entity_id_to_proxy_id_map_[proxy_id_to_entity_id_map_[old_proxy_id]] =
          proxy_id;

      pending_proxy_ids_->Add(proxy_id);
      pending_proxy_local_data_->PushBack(proxy_local_datas_[proxy_id]);

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

void RenderProxyHandler::UpdateSkinningMatrices(
    const frame::SkinningBindings* bindings,
    const frame::MatrixPalettes* palettes) {
  COMET_PROFILE("RenderProxyHandler::UpdateSkinningMatrices");
  auto entity_count{bindings->GetSize()};
  COMET_ASSERT(
      entity_count == palettes->GetSize(),
      "Skinning binding count and matrix palette count should be the same!");

  if (entity_count == 0) {
    return;
  }

  auto total_joint_count{0};

  for (usize i{0}; i < entity_count; ++i) {
    auto& binding{bindings->Get(i)};
    auto skinning_offset{total_joint_count};
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
      pending_proxy_local_data_->PushBack(proxy_local_datas_[proxy_id]);
    }
  }

  if (total_joint_count == 0) {
    return;
  }

  auto skinning_matrix_size{total_joint_count * sizeof(math::Mat4)};

  ReallocateBuffer(
      ssbo_matrix_palettes_, context_->GetAllocatorHandle(),
      skinning_matrix_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_matrix_palettes_");

  MapBuffer(ssbo_matrix_palettes_);
  auto* memory{static_cast<u8*>(ssbo_matrix_palettes_.mapped_memory)};
  sptrdiff cursor{0};

  for (usize i{0}; i < entity_count; ++i) {
    const auto& palette{palettes->Get(i)};
    auto size{palette.skinning_matrix_count * sizeof(math::Mat4)};
    memory::CopyMemory(memory + cursor, palette.skinning_matrices, size);
    cursor += size;
  }

  UnmapBuffer(ssbo_matrix_palettes_);
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
  ReallocateBuffer(
      ssbo_debug_aabbs_, context_->GetAllocatorHandle(),
      render_proxy_count_ * sizeof(GpuDebugAabb),
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_debug_aabbs_");
#endif  // COMET_DEBUG_CULLING

  auto ssbo_proxy_ids_buffer_size{proxy_local_datas_.GetSize() *
                                  sizeof(RenderProxyId)};

  ReallocateBuffer(
      ssbo_proxy_ids_, context_->GetAllocatorHandle(),
      ssbo_proxy_ids_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_proxy_ids_");

  if (pending_proxy_local_data_->GetSize() >
      proxy_local_datas_.GetSize() * kReuploadAllLocalDataThreshold_) {
    UploadAllRenderProxyLocalData();
  } else {
    UploadPendingRenderProxyLocalData();
  }
}

void RenderProxyHandler::UploadAllRenderProxyLocalData() {
  COMET_PROFILE("RenderProxyHandler::UploadAllRenderProxyLocalData");
  auto* allocator_handle{context_->GetAllocatorHandle()};
  auto ssbo_proxy_local_datas_buffer_size{proxy_local_datas_.GetSize() *
                                          sizeof(GpuRenderProxyLocalData)};

  ReallocateBuffer(
      staging_ssbo_proxy_local_datas_, allocator_handle,
      ssbo_proxy_local_datas_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "staging_ssbo_proxy_local_data_");

  MapBuffer(staging_ssbo_proxy_local_datas_);
  auto* memory{staging_ssbo_proxy_local_datas_.mapped_memory};
  memory::CopyMemory(memory, proxy_local_datas_.GetData(),
                     ssbo_proxy_local_datas_buffer_size);
  UnmapBuffer(staging_ssbo_proxy_local_datas_);

  ReallocateBuffer(
      ssbo_proxy_local_datas_, allocator_handle,
      ssbo_proxy_local_datas_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_proxy_local_datas_");

  VkBufferCopy full_copy{};
  full_copy.dstOffset = 0;
  full_copy.size = ssbo_proxy_local_datas_buffer_size;
  full_copy.srcOffset = 0;

  vkCmdCopyBuffer(context_->GetFrameData().command_buffer_handle,
                  staging_ssbo_proxy_local_datas_.handle,
                  ssbo_proxy_local_datas_.handle, 1, &full_copy);

  AddBufferMemoryBarrier(
      ssbo_proxy_local_datas_, post_update_barriers_,
      VK_ACCESS_TRANSFER_WRITE_BIT,
      VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
}

void RenderProxyHandler::UploadPendingRenderProxyLocalData() {
  COMET_PROFILE("RenderProxyHandler::UploadPendingRenderProxyLocalData");
  auto pending_count{pending_proxy_ids_->GetSize()};
  auto* allocator_handle{context_->GetAllocatorHandle()};
  auto ssbo_proxy_local_datas_buffer_size{proxy_local_datas_.GetSize() *
                                          sizeof(GpuRenderProxyLocalData)};

  auto& device{context_->GetDevice()};

  // Resize the buffer if necessary to preserve existing data, instead of
  // reallocating it.
  ResizeBuffer(
      ssbo_proxy_local_datas_, device,
      context_->GetFrameData().command_pool_handle, allocator_handle,
      ssbo_proxy_local_datas_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, device.GetGraphicsQueueHandle(), 0, 0,
      VK_SHARING_MODE_EXCLUSIVE, VK_NULL_HANDLE, "ssbo_proxy_local_datas_");

  // The staging buffer is smaller than or equal to the full buffer size (only
  // pending data is uploaded).
  auto staging_ssbo_proxy_local_datas_buffer_size{
      pending_count * sizeof(GpuRenderProxyLocalData)};

  // From this point, we perform a sparse update on the data at the
  // granularity of shader words. This generic approach lets us update only
  // the modified parts without requiring knowledge of the full structure.
  COMET_ASSERT(
      staging_ssbo_proxy_local_datas_buffer_size % sizeof(ShaderWord) == 0,
      "Data should be a multiple of ShaderWord!");

  ReallocateBuffer(
      staging_ssbo_proxy_local_datas_, allocator_handle,
      staging_ssbo_proxy_local_datas_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "staging_ssbo_proxy_local_data_");

  MapBuffer(staging_ssbo_proxy_local_datas_);

  auto* data_memory{
      static_cast<ShaderWord*>(staging_ssbo_proxy_local_datas_.mapped_memory)};
  memory::CopyMemory(data_memory, pending_proxy_local_data_->GetData(),
                     staging_ssbo_proxy_local_datas_buffer_size);

  UnmapBuffer(staging_ssbo_proxy_local_datas_);

  auto word_count_per_data{static_cast<VkDeviceSize>(
      sizeof(GpuRenderProxyLocalData) / sizeof(ShaderWord))};

  auto ssbo_word_indices_buffer_size{pending_count * sizeof(ShaderWord) *
                                     word_count_per_data};

  ReallocateBuffer(
      ssbo_word_indices_, allocator_handle, ssbo_word_indices_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_word_indices_");

  MapBuffer(ssbo_word_indices_);

  auto* word_indices_memory{
      static_cast<RenderProxyId*>(ssbo_word_indices_.mapped_memory)};

  u32 total_word_count{0};

  for (usize i{0}; i < pending_count; ++i) {
    auto proxy_word_offset{static_cast<ShaderWord>(word_count_per_data *
                                                   pending_proxy_ids_->Get(i))};

    for (u32 word_index{0}; word_index < word_count_per_data; ++word_index) {
      word_indices_memory[total_word_count] = proxy_word_offset + word_index;
      ++total_word_count;
    }
  }

  UnmapBuffer(ssbo_word_indices_);

  ShaderStoragesUpdate update{};
  update.ssbo_word_indices_handle = ssbo_word_indices_.handle;
  update.ssbo_word_indices_size = ssbo_word_indices_.size;

  update.ssbo_source_words_handle = staging_ssbo_proxy_local_datas_.handle;
  update.ssbo_source_words_size = staging_ssbo_proxy_local_datas_.size;

  update.ssbo_destination_words_handle = ssbo_proxy_local_datas_.handle;
  update.ssbo_destination_words_size = ssbo_proxy_local_datas_.size;

  shader_handler_->UpdateConstants(sparse_upload_shader_,
                                   {nullptr, &total_word_count});
  shader_handler_->UpdateStorages(sparse_upload_shader_, update);
  shader_handler_->Bind(sparse_upload_shader_, PipelineBindType::Compute);

  vkCmdDispatch(
      context_->GetFrameData().command_buffer_handle,
      static_cast<u32>(total_word_count + rendering::kShaderLocalSize - 1) /
          rendering::kShaderLocalSize,
      1, 1);
}

void RenderProxyHandler::CommitUpdate(frame::FramePacket* packet) {
  COMET_PROFILE("RenderProxyHandler::CommitUpdate");
  packet->rendering_data = &storages_update_;

  storages_update_.ssbo_proxy_local_datas_handle =
      ssbo_proxy_local_datas_.handle;
  storages_update_.ssbo_proxy_local_datas_size = ssbo_proxy_local_datas_.size;

  storages_update_.ssbo_proxy_ids_handle = ssbo_proxy_ids_.handle;
  storages_update_.ssbo_proxy_ids_size = ssbo_proxy_ids_.size;

  storages_update_.ssbo_proxy_instances_handle = ssbo_proxy_instances_.handle;
  storages_update_.ssbo_proxy_instances_size = ssbo_proxy_instances_.size;

  storages_update_.ssbo_indirect_proxies_handle = ssbo_indirect_proxies_.handle;
  storages_update_.ssbo_indirect_proxies_size = ssbo_indirect_proxies_.size;

  storages_update_.ssbo_source_words_handle = VK_NULL_HANDLE;
  storages_update_.ssbo_destination_words_handle = VK_NULL_HANDLE;

  storages_update_.ssbo_source_words_size = 0;
  storages_update_.ssbo_destination_words_size = 0;

  storages_update_.ssbo_matrix_palettes_handle = ssbo_matrix_palettes_.handle;
  storages_update_.ssbo_matrix_palettes_size = ssbo_matrix_palettes_.size;

#ifdef COMET_DEBUG_RENDERING
  auto& ssbo_debug_data{ssbo_debug_data_[GetDebugDataWriteIndex()]};
  storages_update_.ssbo_debug_data_handle = ssbo_debug_data.handle;
  storages_update_.ssbo_debug_data_size = ssbo_debug_data.size;
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  storages_update_.ssbo_debug_aabbs_handle = ssbo_debug_aabbs_.handle;
  storages_update_.ssbo_debug_aabbs_size = ssbo_debug_aabbs_.size;

  storages_update_.ssbo_debug_lines_handle = VK_NULL_HANDLE;
  storages_update_.ssbo_debug_lines_size = 0;
#endif  // COMET_DEBUG_CULLING
}

void RenderProxyHandler::PrepareRenderProxyDrawData() {
  COMET_PROFILE("RenderProxyHandler::PrepareRenderProxyDrawData");
  ReallocateRenderProxyDrawBuffers();
  PopulateRenderProxyDrawData();
  UploadRenderDrawData();
}

void RenderProxyHandler::ReallocateRenderProxyDrawBuffers() {
  COMET_PROFILE("RenderProxyHandler::ReallocateRenderProxyDrawBuffers");

  if (indirect_batches_->IsEmpty()) {
    return;
  }

  auto* allocator_handle{context_->GetAllocatorHandle()};

  auto ssbo_indirect_proxies_buffer_size{indirect_batches_->GetSize() *
                                         sizeof(GpuIndirectRenderProxy)};

  ReallocateBuffer(staging_ssbo_indirect_proxies_, allocator_handle,
                   ssbo_indirect_proxies_buffer_size,
                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                       VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                   VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
                   "staging_ssbo_indirect_proxies_");

  ReallocateBuffer(ssbo_indirect_proxies_, allocator_handle,
                   ssbo_indirect_proxies_buffer_size,
                   VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                       VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                   VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
                   "ssbo_indirect_proxies_");

  auto ssbo_proxy_instances_buffer_size{sizeof(GpuRenderProxyInstance) *
                                        batch_entries_.GetSize()};

  ReallocateBuffer(
      staging_ssbo_proxy_instances_, allocator_handle,
      ssbo_proxy_instances_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "staging_ssbo_proxy_instances_");

  ReallocateBuffer(
      ssbo_proxy_instances_, allocator_handle, ssbo_proxy_instances_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_proxy_instances_");
}

void RenderProxyHandler::PopulateRenderProxyDrawData() {
  COMET_PROFILE("RenderProxyHandler::PopulateRenderProxyDrawData");

  if (indirect_batches_->IsEmpty()) {
    return;
  }

  MapBuffer(staging_ssbo_indirect_proxies_);
  MapBuffer(staging_ssbo_proxy_instances_);

  auto* indirect_proxies_memory{static_cast<GpuIndirectRenderProxy*>(
      staging_ssbo_indirect_proxies_.mapped_memory)};

  auto* proxy_instances_memory{static_cast<GpuRenderProxyInstance*>(
      staging_ssbo_proxy_instances_.mapped_memory)};

  usize proxy_instance_index{0};

  for (usize batch_id{0}; batch_id < indirect_batches_->GetSize(); ++batch_id) {
    PopulateRenderIndirectProxy(static_cast<BatchId>(batch_id),
                                indirect_proxies_memory);
    PopulateProxyInstances(static_cast<BatchId>(batch_id),
                           proxy_instances_memory, proxy_instance_index);
  }

  UnmapBuffer(staging_ssbo_indirect_proxies_);
  UnmapBuffer(staging_ssbo_proxy_instances_);
}

void RenderProxyHandler::UploadRenderDrawData() {
  COMET_PROFILE("RenderProxyHandler::UploadRenderDrawData");

  if (indirect_batches_->IsEmpty()) {
    return;
  }

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};

  VkBufferCopy indirect_proxies_copy{};
  indirect_proxies_copy.dstOffset = 0;
  indirect_proxies_copy.size = staging_ssbo_indirect_proxies_.size;
  indirect_proxies_copy.srcOffset = 0;

  AddBufferMemoryBarrier(ssbo_indirect_proxies_, shader_to_transfer_barriers_,
                         VK_ACCESS_SHADER_READ_BIT,
                         VK_ACCESS_TRANSFER_WRITE_BIT);

  ApplyBufferMemoryBarriers(
      &shader_to_transfer_barriers_, command_buffer_handle,
      VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

  vkCmdCopyBuffer(command_buffer_handle, staging_ssbo_indirect_proxies_.handle,
                  ssbo_indirect_proxies_.handle, 1, &indirect_proxies_copy);

  AddBufferMemoryBarrier(staging_ssbo_indirect_proxies_, post_update_barriers_,
                         VK_ACCESS_TRANSFER_WRITE_BIT,
                         VK_ACCESS_INDIRECT_COMMAND_READ_BIT);

  AddBufferMemoryBarrier(
      ssbo_indirect_proxies_, post_update_barriers_,
      VK_ACCESS_TRANSFER_WRITE_BIT,
      VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_SHADER_READ_BIT);

  VkBufferCopy proxy_instances_copy{};
  proxy_instances_copy.srcOffset = 0;
  proxy_instances_copy.dstOffset = 0;
  proxy_instances_copy.size = staging_ssbo_proxy_instances_.size;

  vkCmdCopyBuffer(command_buffer_handle, staging_ssbo_proxy_instances_.handle,
                  ssbo_proxy_instances_.handle, 1, &proxy_instances_copy);

  AddBufferMemoryBarrier(ssbo_proxy_instances_, post_update_barriers_,
                         VK_ACCESS_TRANSFER_WRITE_BIT,
                         VK_ACCESS_SHADER_READ_BIT);
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

void RenderProxyHandler::RegisterModelProxy(entity::EntityId model_entity_id,
                                            RenderProxyId proxy_id) {
  auto* proxies{model_to_proxies_map_.TryGet(model_entity_id)};

  if (proxies == nullptr) {
    proxies =
        &model_to_proxies_map_.Emplace(model_entity_id, &general_allocator_)
             .value;
  }

  proxies->proxy_ids.PushBack(proxy_id);
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

u64 RenderProxyHandler::GenerateRenderProxySortKey(const RenderProxy& proxy) {
  auto shader_hash{GenerateHash(proxy.mat_id)};
  auto mesh_material_hash{
      HashCombine(GenerateHash(proxy.mat_id), GenerateHash(proxy.mesh_handle))};
  return static_cast<u64>(shader_hash) << 32 | mesh_material_hash;
}

void RenderProxyHandler::InitializeBuffers() {
  auto allocator_handle{context_->GetAllocatorHandle()};

  staging_ssbo_proxy_local_datas_ = GenerateBuffer(
      allocator_handle, kMaxRenderProxyCount_ * sizeof(GpuRenderProxyLocalData),
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "staging_ssbo_proxy_local_data_");

  ssbo_proxy_local_datas_ = GenerateBuffer(
      allocator_handle, kMaxRenderProxyCount_ * sizeof(GpuRenderProxyLocalData),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_proxy_local_datas_");

  staging_ssbo_indirect_proxies_ = GenerateBuffer(
      allocator_handle, kMaxRenderProxyCount_ * sizeof(GpuIndirectRenderProxy),
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
          VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "staging_ssbo_indirect_proxies_");

  ssbo_indirect_proxies_ = GenerateBuffer(
      allocator_handle, kMaxRenderProxyCount_ * sizeof(GpuIndirectRenderProxy),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
          VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_indirect_proxies_");

  ssbo_proxy_ids_ = GenerateBuffer(
      allocator_handle, kMaxRenderProxyCount_ * sizeof(RenderProxyId),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_proxy_ids_");

  staging_ssbo_proxy_instances_ = GenerateBuffer(
      allocator_handle, kMaxRenderProxyCount_ * sizeof(GpuRenderProxyInstance),
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "staging_ssbo_proxy_instances_");

  ssbo_proxy_instances_ = GenerateBuffer(
      allocator_handle, kMaxRenderProxyCount_ * sizeof(GpuRenderProxyInstance),
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_proxy_instances_");

  ssbo_matrix_palettes_ = GenerateBuffer(
      allocator_handle,
      static_cast<VkDeviceSize>(kMaxRenderProxyCount_ * .1f) * 100 *
          sizeof(math::Mat4),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_matrix_palettes_");

  auto word_count_per_data{static_cast<VkDeviceSize>(
      sizeof(GpuRenderProxyLocalData) / sizeof(ShaderWord))};

  auto ssbo_proxy_ids_buffer_size{kMaxRenderProxyCount_ * sizeof(ShaderWord) *
                                  word_count_per_data};

  ssbo_word_indices_ = GenerateBuffer(
      allocator_handle, ssbo_proxy_ids_buffer_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_word_indices_");
}

void RenderProxyHandler::DestroyBuffers() {
  if (IsBufferInitialized(staging_ssbo_proxy_local_datas_)) {
    DestroyBuffer(staging_ssbo_proxy_local_datas_);
  }

  if (IsBufferInitialized(ssbo_proxy_local_datas_)) {
    DestroyBuffer(ssbo_proxy_local_datas_);
  }

  if (IsBufferInitialized(staging_ssbo_indirect_proxies_)) {
    DestroyBuffer(staging_ssbo_indirect_proxies_);
  }

  if (IsBufferInitialized(ssbo_indirect_proxies_)) {
    DestroyBuffer(ssbo_indirect_proxies_);
  }

  if (IsBufferInitialized(ssbo_proxy_ids_)) {
    DestroyBuffer(ssbo_proxy_ids_);
  }

  if (IsBufferInitialized(staging_ssbo_proxy_instances_)) {
    DestroyBuffer(staging_ssbo_proxy_instances_);
  }

  if (IsBufferInitialized(ssbo_proxy_instances_)) {
    DestroyBuffer(ssbo_proxy_instances_);
  }

  if (IsBufferInitialized(ssbo_matrix_palettes_)) {
    DestroyBuffer(ssbo_matrix_palettes_);
  }

  if (IsBufferInitialized(ssbo_word_indices_)) {
    DestroyBuffer(ssbo_word_indices_);
  }
}

#ifdef COMET_DEBUG_RENDERING
void RenderProxyHandler::InitializeDebugData() {
  auto* allocator_handle{context_->GetAllocatorHandle()};

  for (u32 i{0}; i < kDebugDataBufferCount_; ++i) {
    auto& ssbo_debug_data{ssbo_debug_data_[i]};

    ReallocateBuffer(
        ssbo_debug_data, allocator_handle, sizeof(GpuDebugData),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_TO_CPU, 0, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE, "debug_data_");
    MapBuffer(ssbo_debug_data);
    debug_data_[i] = static_cast<GpuDebugData*>(ssbo_debug_data.mapped_memory);
  }
}

void RenderProxyHandler::DestroyDebugData() {
  for (u32 i{0}; i < kDebugDataBufferCount_; ++i) {
    auto& ssbo_debug_data{ssbo_debug_data_[i]};

    if (IsBufferInitialized(ssbo_debug_data)) {
      UnmapBuffer(ssbo_debug_data);
      DestroyBuffer(ssbo_debug_data);
    }
  }
}

usize RenderProxyHandler::GetDebugDataReadIndex() const {
  return (context_->GetFrameCount() + kDebugDataBufferCount_ - 1) %
         kDebugDataBufferCount_;
}

usize RenderProxyHandler::GetDebugDataWriteIndex() const {
  return context_->GetFrameCount() % kDebugDataBufferCount_;
}
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
void RenderProxyHandler::InitializeCullingDebug() {
  ShaderDescr shader_descr{};
  shader_descr.resource_path =
      COMET_TCHAR("shaders/vulkan/debug_shader.vk.cshader");
  auto* allocator_handle{context_->GetAllocatorHandle()};

  ReallocateBuffer(
      ssbo_debug_aabbs_, allocator_handle,
      kDefaultProxyCount_ * sizeof(GpuDebugAabb),
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_debug_aabbs_");

  ReallocateBuffer(
      ssbo_debug_lines_, allocator_handle,
      kDefaultProxyCount_ * 24 * sizeof(math::Vec3),
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_debug_lines_");
}

void RenderProxyHandler::DestroyCullingDebug() {
  if (IsBufferInitialized(ssbo_debug_aabbs_)) {
    DestroyBuffer(ssbo_debug_aabbs_);
  }

  if (IsBufferInitialized(ssbo_debug_lines_)) {
    DestroyBuffer(ssbo_debug_lines_);
  }
}
#endif  // COMET_DEBUG_CULLING
}  // namespace vk
}  // namespace rendering
}  // namespace comet
