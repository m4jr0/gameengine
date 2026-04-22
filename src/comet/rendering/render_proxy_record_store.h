// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDER_PROXY_RECORD_STORE_H_
#define COMET_COMET_RENDERING_RENDER_PROXY_RECORD_STORE_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/map.h"
#include "comet/entity/type/entity_id.h"
#include "comet/rendering/type/render_proxy.h"
#include "comet/resource/material/material_resource.h"

namespace comet {
namespace rendering {
class RenderProxyRecordStore {
 public:
  RenderProxyRecordStore() = default;
  RenderProxyRecordStore(const RenderProxyRecordStore&) = delete;
  RenderProxyRecordStore(RenderProxyRecordStore&&) = delete;
  RenderProxyRecordStore& operator=(const RenderProxyRecordStore&) = delete;
  RenderProxyRecordStore& operator=(RenderProxyRecordStore&&) = delete;
  ~RenderProxyRecordStore() = default;

  void Initialize(usize expected_proxy_count);
  void Shutdown();

  void Update(const frame::FramePacket* packet);
  void ResetFrameState();

  usize GetRenderProxyCount() const noexcept;
  bool IsEmpty() const noexcept;

  bool IsAlive(RenderProxyId proxy_id) const noexcept;
  bool HasProxy(entity::EntityId entity_id) const noexcept;

  RenderProxyId GetProxyId(entity::EntityId entity_id) const;
  const RenderProxyRecord* TryGetRecord(RenderProxyId proxy_id) const noexcept;
  RenderProxyRecord* TryGetRecord(RenderProxyId proxy_id) noexcept;

  const RenderProxyRecord& GetRecord(RenderProxyId proxy_id) const;
  RenderProxyRecord& GetRecord(RenderProxyId proxy_id);

  usize GetRecordSlotCount() const noexcept;
  const Array<RenderProxyId>& GetActiveProxyIds() const noexcept;
  const Array<RenderProxyId>& GetDirtyProxyIds() const noexcept;
  const Array<RenderProxyId>& GetAddedProxyIds() const noexcept;
  const Array<RenderProxyId>& GetRemovedProxyIds() const noexcept;

  const Map<entity::EntityId, RenderProxyModelBindings>& GetModelBindings()
      const noexcept;

 private:
  static constexpr auto kDefaultProxyCount_{512};

  void DestroyRenderProxies(const frame::RemovedGeometries* geometries);
  void GenerateRenderProxies(const frame::AddedGeometries* geometries);
  void UpdateRenderProxies(const frame::DirtyMeshes* meshes,
                           const frame::DirtyTransforms* transforms);
  void UpdateSkinningMatrices(const frame::SkinningBindings* bindings,
                              const frame::MatrixPalettes* palettes);

  RenderProxyId AllocateProxyId();
  void ReleaseProxyId(RenderProxyId proxy_id);

  void MarkDirty(RenderProxyId proxy_id, RenderProxyDirtyFlag dirty_flag);
  void ClearDirty(RenderProxyId proxy_id);

  void RegisterModelProxy(entity::EntityId model_entity_id,
                          RenderProxyId proxy_id);
  void UnregisterModelProxy(entity::EntityId model_entity_id,
                            RenderProxyId proxy_id);

#ifdef COMET_DEBUG
  void AssertInvariants() const;
#endif  // COMET_DEBUG

  memory::FiberFreeListAllocator record_allocator_{
      sizeof(RenderProxyRecord), kDefaultProxyCount_,
      memory::kEngineMemoryTagRendering};

  memory::FiberFreeListAllocator id_allocator_{
      sizeof(RenderProxyId) * 64, kDefaultProxyCount_,
      memory::kEngineMemoryTagRendering};

  memory::FiberFreeListAllocator map_allocator_{
      256, kDefaultProxyCount_, memory::kEngineMemoryTagRendering};

  Array<RenderProxyRecord> record_slots_{};
  Array<RenderProxyId> active_proxy_ids_{};
  Array<RenderProxyId> free_proxy_ids_{};

  Array<RenderProxyId> dirty_proxy_ids_{};
  Array<RenderProxyId> added_proxy_ids_{};
  Array<RenderProxyId> removed_proxy_ids_{};

  Map<entity::EntityId, RenderProxyId> entity_id_to_proxy_id_map_{};
  Map<entity::EntityId, RenderProxyModelBindings> model_to_proxies_map_{};

  usize render_proxy_count_{0};
};
}  // namespace rendering
}  // namespace comet

#endif  // !COMET_COMET_RENDERING_RENDER_PROXY_RECORD_STORE_H_