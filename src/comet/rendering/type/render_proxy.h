// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_TYPE_RENDER_PROXY_H_
#define COMET_COMET_RENDERING_TYPE_RENDER_PROXY_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/entity/type/entity_id.h"
#include "comet/geometry/type/mesh.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/rendering/rendering_handle.h"

namespace comet {
namespace rendering {
using RenderProxyId = u32;
constexpr auto kInvalidRenderProxyId{static_cast<RenderProxyId>(-1)};

using BatchId = u32;
constexpr auto kInvalidBatchId{static_cast<BatchId>(-1)};

using SkinningOffset = u32;
constexpr auto kInvalidSkinningOffset{static_cast<SkinningOffset>(-1)};

struct RenderProxy {
  RenderProxyId id{kInvalidRenderProxyId};
  entity::EntityId entity_id{entity::kInvalidEntityId};
  entity::EntityId model_entity_id{entity::kInvalidEntityId};
  geometry::MeshHandle mesh_handle{};
};

struct RenderProxyModelBindings {
  Array<RenderProxyId> proxy_ids{};
};

struct RenderBatchEntry {
  using SortKey = u64;
  static inline constexpr auto kInvalidSortKey{static_cast<SortKey>(-1)};

  SortKey sort_key{kInvalidSortKey};
  RenderProxyId proxy_id{kInvalidRenderProxyId};
};

struct RenderIndirectBatch {
  u32 offset{0};
  u32 count{0};
  RenderProxyId proxy_id{kInvalidRenderProxyId};
};

struct RenderBatchGroup {
  u32 offset{0};
  u32 count{0};
};

struct GpuRenderProxyLocalData {
  math::Vec4 local_center{.0f};
  math::Vec4 local_max_extents{.0f};
  math::Mat4 transform{1.0f};
  SkinningOffset skinning_offset{kInvalidSkinningOffset};
  SkinningOffset padding[3]{};
};

struct GpuRenderProxyInstance {
  RenderProxyId proxy_id{kInvalidRenderProxyId};
  BatchId batch_id{kInvalidBatchId};
};

#ifdef COMET_DEBUG_RENDERING
struct GpuDebugData {
  u32 visible_count{0};
};
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
struct GpuDebugAabb {
  math::Vec4 min_extents{.0f};
  math::Vec4 max_extents{.0f};
};
#endif  // COMET_DEBUG_CULLING

enum class RenderProxyDirtyFlag : u8 {
  None = 0x0,
  LocalData = 0x1,
  Skinning = 0x2,
  Added = 0x4,
  Removed = 0x8,
};

struct RenderProxyRecord {
  bool is_alive{false};
  bool is_dirty{false};
  RenderProxyDirtyFlag dirty_flags{RenderProxyDirtyFlag::None};

  RenderProxy proxy{};
  GpuRenderProxyLocalData local_data{};

  resource::MaterialResourceId material_resource_id{};
};
}  // namespace rendering
}  // namespace comet

#endif  // !COMET_COMET_RENDERING_TYPE_RENDER_PROXY_H_