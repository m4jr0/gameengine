// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_FRAME_PACKET_H_
#define COMET_COMET_CORE_FRAME_PACKET_H_

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/essentials.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/hash.h"
#include "comet/core/type/array.h"
#include "comet/core/type/ordered_set.h"
#include "comet/entity/entity_id.h"
#include "comet/geometry/component/mesh_component.h"
#include "comet/geometry/geometry_common.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/physics/component/transform_component.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace frame {
using FrameCount = usize;

enum FrameStage { Unknown = -1, Game = 0, Render = 1, Gpu = 2, Flip = 3 };
using FrameStageTimestamp = usize;
constexpr auto kInvalidFrameStageTimestamp{
    static_cast<FrameStageTimestamp>(-1)};

// TODO(m4jr0): Implement stage times.
struct StageTimes {
  FrameStageTimestamp start{kInvalidFrameStageTimestamp};
  FrameStageTimestamp end{kInvalidFrameStageTimestamp};
};

constexpr auto kFrameStageCount{4};

#ifdef COMET_DEBUG
using FramePacketDebugId = usize;
constexpr auto kInvalidFramePacketDebugId{static_cast<FramePacketDebugId>(-1)};
#endif  // COMET_DEBUG

struct AddedGeometry {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  geometry::MeshId mesh_id{geometry::kInvalidMeshId};
  const resource::MaterialResource* material{nullptr};
  DoubleFrameArray<geometry::Index>* indices{};
  DoubleFrameArray<geometry::Vertex>* vertices{};
  math::Mat4 transform{1.0f};
  math::Vec3 local_center{0.0f};
  math::Vec3 local_max_extents{0.0f};
};

struct DirtyMesh {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  geometry::MeshId mesh_id{geometry::kInvalidMeshId};
  const resource::MaterialResource* material{nullptr};
  math::Vec3 local_center{0.0f};
  math::Vec3 local_max_extents{0.0f};
  DoubleFrameArray<geometry::Index>* indices{nullptr};
  DoubleFrameArray<geometry::Vertex>* vertices{nullptr};
};

struct DirtyTransform {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  math::Mat4 transform{1.0f};
};

struct RemovedGeometry {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  geometry::MeshId mesh_id{geometry::kInvalidMeshId};
};

HashValue GenerateHash(const AddedGeometry& value);
HashValue GenerateHash(const DirtyMesh& value);
HashValue GenerateHash(const DirtyTransform& value);
HashValue GenerateHash(const RemovedGeometry& value);

using AddedGeometries = frame::DoubleFrameOrderedSet<AddedGeometry>;
using DirtyMeshes = frame::DoubleFrameOrderedSet<DirtyMesh>;
using DirtyTransforms = frame::DoubleFrameOrderedSet<DirtyTransform>;
using RemovedGeometries = frame::DoubleFrameOrderedSet<RemovedGeometry>;

struct FramePacket {
#ifdef COMET_DEBUG
 private:
  inline static FramePacketDebugId debug_id_counter_{0};

 public:
  FramePacketDebugId debug_id{FramePacket::debug_id_counter_++};
#endif  // COMET_DEBUG
  void RegisterNewGeometry(entity::EntityId entity_id,
                           const geometry::MeshComponent* mesh_cmp,
                           const physics::TransformComponent* transform_cmp);

  void RegisterDirtyMesh(entity::EntityId entity_id,
                         const geometry::MeshComponent* mesh_cmp);

  void RegisterDirtyTransform(entity::EntityId entity_id,
                              const physics::TransformComponent* transform_cmp);

  void RegisterRemovedGeometry(entity::EntityId entity_id,
                               const geometry::MeshComponent* mesh_cmp);

  bool IsFrameStageStarted(FrameStage stage) const;
  bool IsFrameStageFinished(FrameStage stage) const;
  void Reset();

  FrameCount frame_count{0};
  f64 lag{.0f};
  time::Interpolation interpolation{.0f};
  // TODO(m4jr0): Add skinning matrices.
  // TODO(m4jr0): Add list of meshes to render.
  StageTimes stage_times[kFrameStageCount]{};
  math::Mat4 projection_matrix{};
  math::Mat4 view_matrix{};
  usize draw_count{0};

  AddedGeometries* added_geometries{nullptr};
  DirtyMeshes* dirty_meshes{nullptr};
  DirtyTransforms* dirty_transforms{nullptr};
  RemovedGeometries* removed_geometries{nullptr};

  void* rendering_data{nullptr};

 private:
  fiber::FiberMutex added_geometries_mtx{};
  fiber::FiberMutex dirty_meshes_mtx{};
  fiber::FiberMutex dirty_transforms_mtx{};
  fiber::FiberMutex removed_geometries_mtx{};
};
}  // namespace frame
}  // namespace comet

#endif  // COMET_COMET_CORE_FRAME_PACKET_H_
