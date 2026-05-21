// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/render_proxy_record_store.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/logger/logging.h"
#include "comet/entity/type/entity_id.h"
#include "comet/math/vector.h"

namespace comet {
namespace rendering {
namespace {
constexpr auto operator|(RenderProxyDirtyFlag lhs,
                         RenderProxyDirtyFlag rhs) noexcept
    -> RenderProxyDirtyFlag {
  return static_cast<RenderProxyDirtyFlag>(static_cast<u8>(lhs) |
                                           static_cast<u8>(rhs));
}
}  // namespace

void RenderProxyRecordStore::Initialize(usize expected_proxy_count) {
  record_allocator_.Initialize();
  id_allocator_.Initialize();
  map_allocator_.Initialize();

  const auto capacity{expected_proxy_count == 0 ? kDefaultProxyCount_
                                                : expected_proxy_count};

  record_slots_ =
      Array<RenderProxyRecord>::WithCapacity(&record_allocator_, capacity);

  active_proxy_ids_ =
      Array<RenderProxyId>::WithCapacity(&id_allocator_, capacity);
  free_proxy_ids_ =
      Array<RenderProxyId>::WithCapacity(&id_allocator_, capacity);

  dirty_proxy_ids_ =
      Array<RenderProxyId>::WithCapacity(&id_allocator_, capacity);
  added_proxy_ids_ =
      Array<RenderProxyId>::WithCapacity(&id_allocator_, capacity);
  removed_proxy_ids_ =
      Array<RenderProxyId>::WithCapacity(&id_allocator_, capacity);

  entity_id_to_proxy_id_map_ =
      Map<entity::EntityId, RenderProxyId>::WithCapacity(&map_allocator_,
                                                         capacity);
  model_to_proxies_map_ =
      Map<entity::EntityId, RenderProxyModelBindings>::WithCapacity(
          &map_allocator_, capacity);

  render_proxy_count_ = 0;
}

void RenderProxyRecordStore::Shutdown() {
  for (auto& pair : model_to_proxies_map_) {
    pair.value.proxy_ids.Release();
  }

  model_to_proxies_map_.Release();
  entity_id_to_proxy_id_map_.Release();

  removed_proxy_ids_.Release();
  added_proxy_ids_.Release();
  dirty_proxy_ids_.Release();

  free_proxy_ids_.Release();
  active_proxy_ids_.Release();
  record_slots_.Release();

  render_proxy_count_ = 0;

  map_allocator_.Destroy();
  id_allocator_.Destroy();
  record_allocator_.Destroy();
}

void RenderProxyRecordStore::Update(const frame::FramePacket* packet) {
  COMET_ASSERT(packet != nullptr, "RenderProxyRecordStore::Update",
               "frame packet is null");

  ResetFrameState();

  DestroyRenderProxies(packet->removed_geometries);
  GenerateRenderProxies(packet->added_geometries);
  UpdateRenderProxies(packet->dirty_meshes, packet->dirty_transforms);
  UpdateSkinningMatrices(packet->skinning_bindings, packet->matrix_palettes);

#ifdef COMET_DEBUG
  AssertInvariants();
#endif  // COMET_DEBUG
}

void RenderProxyRecordStore::ResetFrameState() {
  for (const auto proxy_id : dirty_proxy_ids_) {
    if (proxy_id >= record_slots_.GetSize()) {
      continue;
    }

    auto& record{record_slots_[proxy_id]};
    record.is_dirty = false;
    record.dirty_flags = RenderProxyDirtyFlag::None;
  }

  dirty_proxy_ids_.Clear();
  added_proxy_ids_.Clear();
  removed_proxy_ids_.Clear();
}

usize RenderProxyRecordStore::GetRenderProxyCount() const noexcept {
  return render_proxy_count_;
}

bool RenderProxyRecordStore::IsEmpty() const noexcept {
  return render_proxy_count_ == 0;
}

bool RenderProxyRecordStore::IsAlive(RenderProxyId proxy_id) const noexcept {
  const auto* record{TryGetRecord(proxy_id)};
  return record != nullptr && record->is_alive;
}

bool RenderProxyRecordStore::HasProxy(
    entity::EntityId entity_id) const noexcept {
  return entity_id_to_proxy_id_map_.IsContained(entity_id);
}

RenderProxyId RenderProxyRecordStore::GetProxyId(
    entity::EntityId entity_id) const {
  const auto* proxy_id{entity_id_to_proxy_id_map_.TryGet(entity_id)};
  COMET_ASSERT(proxy_id != nullptr, "RenderProxyRecordStore::GetProxyId",
               "render proxy id not found", "entity_id", entity_id);
  return *proxy_id;
}

const RenderProxyRecord* RenderProxyRecordStore::TryGetRecord(
    RenderProxyId proxy_id) const noexcept {
  if (proxy_id >= record_slots_.GetSize()) {
    return nullptr;
  }

  const auto& record{record_slots_[proxy_id]};
  return record.is_alive ? &record : nullptr;
}

RenderProxyRecord* RenderProxyRecordStore::TryGetRecord(
    RenderProxyId proxy_id) noexcept {
  if (proxy_id >= record_slots_.GetSize()) {
    return nullptr;
  }

  auto& record{record_slots_[proxy_id]};
  return record.is_alive ? &record : nullptr;
}

const RenderProxyRecord& RenderProxyRecordStore::GetRecord(
    RenderProxyId proxy_id) const {
  const auto* record{TryGetRecord(proxy_id)};
  COMET_ASSERT(record != nullptr, "RenderProxyRecordStore::GetRecord",
               "render proxy record not found", "proxy_id", proxy_id);
  return *record;
}

RenderProxyRecord& RenderProxyRecordStore::GetRecord(RenderProxyId proxy_id) {
  auto* record{TryGetRecord(proxy_id)};
  COMET_ASSERT(record != nullptr, "RenderProxyRecordStore::GetRecord",
               "render proxy record not found", "proxy_id", proxy_id);
  return *record;
}

usize RenderProxyRecordStore::GetRecordSlotCount() const noexcept {
  return record_slots_.GetSize();
}

const Array<RenderProxyId>& RenderProxyRecordStore::GetActiveProxyIds()
    const noexcept {
  return active_proxy_ids_;
}

const Array<RenderProxyId>& RenderProxyRecordStore::GetDirtyProxyIds()
    const noexcept {
  return dirty_proxy_ids_;
}

const Array<RenderProxyId>& RenderProxyRecordStore::GetAddedProxyIds()
    const noexcept {
  return added_proxy_ids_;
}

const Array<RenderProxyId>& RenderProxyRecordStore::GetRemovedProxyIds()
    const noexcept {
  return removed_proxy_ids_;
}

const Map<entity::EntityId, RenderProxyModelBindings>&
RenderProxyRecordStore::GetModelBindings() const noexcept {
  return model_to_proxies_map_;
}

void RenderProxyRecordStore::DestroyRenderProxies(
    const frame::RemovedGeometries* geometries) {
  COMET_ASSERT(geometries != nullptr,
               "RenderProxyRecordStore::DestroyRenderProxies",
               "removed geometries are null");

  if (geometries->IsEmpty()) {
    return;
  }

  for (const auto& geometry : *geometries) {
    const auto* proxy_id_ptr{
        entity_id_to_proxy_id_map_.TryGet(geometry.entity_id)};

    if (proxy_id_ptr == nullptr) {
      COMET_LOG_WARNING(
          LoggerType::Rendering, "RenderProxyRecordStore::DestroyRenderProxies",
          "render proxy not found", "entity_id", geometry.entity_id);
      continue;
    }

    const auto proxy_id{*proxy_id_ptr};
    auto* record{TryGetRecord(proxy_id)};

    if (record == nullptr) {
      COMET_LOG_WARNING(LoggerType::Rendering,
                        "RenderProxyRecordStore::DestroyRenderProxies",
                        "render proxy record not alive", "proxy_id", proxy_id,
                        "entity_id", geometry.entity_id);
      continue;
    }

    UnregisterModelProxy(record->proxy.model_entity_id, proxy_id);

    entity_id_to_proxy_id_map_.Remove(record->proxy.entity_id);

    record->is_alive = false;
    record->is_dirty = false;
    record->dirty_flags = RenderProxyDirtyFlag::Removed;

    removed_proxy_ids_.PushLast(proxy_id);
    ReleaseProxyId(proxy_id);

    for (usize i{0}; i < active_proxy_ids_.GetSize(); ++i) {
      if (active_proxy_ids_[i] != proxy_id) {
        continue;
      }

      active_proxy_ids_.RemoveFromIndex(i);
      break;
    }

    COMET_ASSERT(render_proxy_count_ != 0,
                 "RenderProxyRecordStore::DestroyRenderProxies",
                 "render proxy count underflow");
    --render_proxy_count_;
  }
}

void RenderProxyRecordStore::GenerateRenderProxies(
    const frame::AddedGeometries* geometries) {
  COMET_ASSERT(geometries != nullptr,
               "RenderProxyRecordStore::GenerateRenderProxies",
               "added geometries are null");

  if (geometries->IsEmpty()) {
    return;
  }

  for (const auto& geometry : *geometries) {
    COMET_ASSERT(!entity_id_to_proxy_id_map_.IsContained(geometry.entity_id),
                 "RenderProxyRecordStore::GenerateRenderProxies",
                 "geometry is already known", "entity_id", geometry.entity_id,
                 "mesh_handle", geometry.mesh_handle);
    const auto proxy_id{AllocateProxyId()};
    auto& record{record_slots_[proxy_id]};

    record.is_alive = true;
    record.is_dirty = true;
    record.dirty_flags =
        RenderProxyDirtyFlag::Added | RenderProxyDirtyFlag::LocalData;

    record.proxy.id = proxy_id;
    record.proxy.entity_id = geometry.entity_id;
    record.proxy.model_entity_id = geometry.model_entity_id;
    record.proxy.mesh_handle = geometry.mesh_handle;

    record.local_data.local_center = math::Vec4{geometry.local_center, .0f};
    record.local_data.local_max_extents =
        math::Vec4{geometry.local_max_extents, .0f};
    record.local_data.transform = geometry.transform;
    record.local_data.skinning_offset = kInvalidSkinningOffset;
    record.local_data.padding[0] = 0;
    record.local_data.padding[1] = 0;
    record.local_data.padding[2] = 0;

    record.material_resource_id = geometry.material_resource_id;

    entity_id_to_proxy_id_map_.Set(geometry.entity_id, proxy_id);
    RegisterModelProxy(geometry.model_entity_id, proxy_id);

    active_proxy_ids_.PushLast(proxy_id);
    dirty_proxy_ids_.PushLast(proxy_id);
    added_proxy_ids_.PushLast(proxy_id);

    ++render_proxy_count_;
  }
}

void RenderProxyRecordStore::UpdateRenderProxies(
    const frame::DirtyMeshes* meshes,
    const frame::DirtyTransforms* transforms) {
  COMET_ASSERT(meshes != nullptr, "RenderProxyRecordStore::UpdateRenderProxies",
               "dirty meshes are null");
  COMET_ASSERT(transforms != nullptr,
               "RenderProxyRecordStore::UpdateRenderProxies",
               "dirty transforms are null");

  for (const auto& updated_mesh : *meshes) {
    const auto* proxy_id_ptr{
        entity_id_to_proxy_id_map_.TryGet(updated_mesh.entity_id)};

    if (proxy_id_ptr == nullptr) {
      continue;
    }

    auto& record{GetRecord(*proxy_id_ptr)};
    record.local_data.local_center = math::Vec4{updated_mesh.local_center, .0f};
    record.local_data.local_max_extents =
        math::Vec4{updated_mesh.local_max_extents, .0f};

    MarkDirty(record.proxy.id, RenderProxyDirtyFlag::LocalData);
  }

  for (const auto& updated_transform : *transforms) {
    const auto* proxy_id_ptr{
        entity_id_to_proxy_id_map_.TryGet(updated_transform.entity_id)};

    if (proxy_id_ptr == nullptr) {
      continue;
    }

    auto& record{GetRecord(*proxy_id_ptr)};
    record.local_data.transform = updated_transform.transform;

    MarkDirty(record.proxy.id, RenderProxyDirtyFlag::LocalData);
  }
}

void RenderProxyRecordStore::UpdateSkinningMatrices(
    const frame::SkinningBindings* bindings,
    const frame::MatrixPalettes* palettes) {
  COMET_ASSERT(bindings != nullptr,
               "RenderProxyRecordStore::UpdateSkinningMatrices",
               "skinning bindings are null");
  COMET_ASSERT(palettes != nullptr,
               "RenderProxyRecordStore::UpdateSkinningMatrices",
               "matrix palettes are null");
  COMET_ASSERT(bindings->GetSize() == palettes->GetSize(),
               "RenderProxyRecordStore::UpdateSkinningMatrices",
               "binding palette count mismatch", "binding_count",
               bindings->GetSize(), "palette_count", palettes->GetSize());

  usize total_joint_count{0};

  for (usize i{0}; i < bindings->GetSize(); ++i) {
    const auto& binding{bindings->Get(i)};
    [[maybe_unused]] const auto& palette{palettes->Get(i)};

    COMET_ASSERT(binding.joint_count == palette.skinning_matrix_count,
                 "RenderProxyRecordStore::UpdateSkinningMatrices",
                 "joint matrix count mismatch", "entity_id", binding.entity_id,
                 "joint_count", binding.joint_count, "matrix_count",
                 palette.skinning_matrix_count);

    const auto skinning_offset{static_cast<SkinningOffset>(total_joint_count)};
    total_joint_count += binding.joint_count;

    if (binding.entity_id == entity::kInvalidEntityId) {
      continue;
    }

    auto* bindings_for_model{model_to_proxies_map_.TryGet(binding.entity_id)};

    if (bindings_for_model == nullptr) {
      continue;
    }

    for (const auto proxy_id : bindings_for_model->proxy_ids) {
      auto& record{GetRecord(proxy_id)};
      record.local_data.skinning_offset = skinning_offset;
      MarkDirty(proxy_id, RenderProxyDirtyFlag::Skinning);
    }
  }
}

RenderProxyId RenderProxyRecordStore::AllocateProxyId() {
  if (!free_proxy_ids_.IsEmpty()) {
    const auto proxy_id{free_proxy_ids_.TakeLast()};
    return proxy_id;
  }

  const auto proxy_id{static_cast<RenderProxyId>(record_slots_.GetSize())};
  record_slots_.Resize(record_slots_.GetSize() + 1);
  return proxy_id;
}

void RenderProxyRecordStore::ReleaseProxyId(RenderProxyId proxy_id) {
  free_proxy_ids_.PushLast(proxy_id);
}

void RenderProxyRecordStore::MarkDirty(RenderProxyId proxy_id,
                                       RenderProxyDirtyFlag dirty_flag) {
  auto& record{GetRecord(proxy_id)};

  if (!record.is_dirty) {
    record.is_dirty = true;
    record.dirty_flags = dirty_flag;
    dirty_proxy_ids_.PushLast(proxy_id);
    return;
  }

  record.dirty_flags = record.dirty_flags | dirty_flag;
}

void RenderProxyRecordStore::ClearDirty(RenderProxyId proxy_id) {
  auto& record{GetRecord(proxy_id)};
  record.is_dirty = false;
  record.dirty_flags = RenderProxyDirtyFlag::None;
}

void RenderProxyRecordStore::RegisterModelProxy(
    entity::EntityId model_entity_id, RenderProxyId proxy_id) {
  auto* bindings{model_to_proxies_map_.TryGet(model_entity_id)};

  if (bindings == nullptr) {
    auto& pair{model_to_proxies_map_.Emplace(model_entity_id,
                                             RenderProxyModelBindings{})};
    pair.value.proxy_ids =
        Array<RenderProxyId>::WithCapacity(&id_allocator_, 4);
    bindings = &pair.value;
  }

  bindings->proxy_ids.PushLast(proxy_id);
}

void RenderProxyRecordStore::UnregisterModelProxy(
    entity::EntityId model_entity_id, RenderProxyId proxy_id) {
  auto* bindings{model_to_proxies_map_.TryGet(model_entity_id)};

  if (bindings == nullptr) {
    return;
  }

  bindings->proxy_ids.RemoveFromValue(proxy_id);

  if (!bindings->proxy_ids.IsEmpty()) {
    return;
  }

  bindings->proxy_ids.Release();
  model_to_proxies_map_.Remove(model_entity_id);
}

#ifdef COMET_DEBUG
void RenderProxyRecordStore::AssertInvariants() const {
  COMET_ASSERT(render_proxy_count_ == active_proxy_ids_.GetSize(),
               "RenderProxyRecordStore::AssertInvariants",
               "active proxy count mismatch", "render_proxy_count",
               render_proxy_count_, "active_proxy_count",
               active_proxy_ids_.GetSize());

  for (const auto proxy_id : active_proxy_ids_) {
    COMET_ASSERT(proxy_id < record_slots_.GetSize(),
                 "RenderProxyRecordStore::AssertInvariants",
                 "active proxy id out of range", "proxy_id", proxy_id,
                 "record_slot_count", record_slots_.GetSize());

    const auto& record{record_slots_[proxy_id]};

    COMET_ASSERT(record.is_alive, "RenderProxyRecordStore::AssertInvariants",
                 "active proxy record is not alive", "proxy_id", proxy_id);

    COMET_ASSERT(record.proxy.id == proxy_id,
                 "RenderProxyRecordStore::AssertInvariants",
                 "proxy id mismatch", "proxy_id", proxy_id, "record_proxy_id",
                 record.proxy.id);

    const auto* mapped_proxy_id{
        entity_id_to_proxy_id_map_.TryGet(record.proxy.entity_id)};

    COMET_ASSERT(mapped_proxy_id != nullptr,
                 "RenderProxyRecordStore::AssertInvariants",
                 "entity to proxy mapping missing", "proxy_id", proxy_id,
                 "entity_id", record.proxy.entity_id);

    COMET_ASSERT(*mapped_proxy_id == proxy_id,
                 "RenderProxyRecordStore::AssertInvariants",
                 "entity to proxy mapping mismatch", "entity_id",
                 record.proxy.entity_id, "expected_proxy_id", proxy_id,
                 "mapped_proxy_id", *mapped_proxy_id);
  }
}
#endif  // COMET_DEBUG
}  // namespace rendering
}  // namespace comet