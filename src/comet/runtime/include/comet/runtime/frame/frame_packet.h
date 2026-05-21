// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RUNTIME_FRAME_FRAME_PACKET_H_
#define COMET_COMET_RUNTIME_FRAME_FRAME_PACKET_H_

#include "comet/animation/type/animation_skinning.h"
#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/frame/frame_container.h"
#include "comet/core/hash.h"
#include "comet/entity/type/entity_id.h"
#include "comet/geometry/component/mesh_component.h"
#include "comet/geometry/type/mesh.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/physics/component/transform_component.h"
#include "comet/rendering/type/camera.h"
#include "comet/rendering/type/light.h"
#include "comet/rendering/type/texture.h"
#include "comet/resource/material/material_resource.h"
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

constexpr usize kFrameStageCount{4};
constexpr usize kMaxCameraViewCount{2};

#ifdef COMET_DEBUG
using FramePacketDebugId = usize;
constexpr auto kInvalidFramePacketDebugId{static_cast<FramePacketDebugId>(-1)};
#endif  // COMET_DEBUG

struct AddedGeometry {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  entity::EntityId model_entity_id{entity::kInvalidEntityId};
  geometry::MeshHandle mesh_handle{};

  // Retained by the frame packet. Released in FramePacket::Reset().
  resource::MaterialResourceId material_resource_id{};

  DoubleFrameArray<geometry::Index>* indices{nullptr};
  DoubleFrameArray<geometry::SkinnedVertex>* vertices{nullptr};

  math::Mat4 transform{1.0f};
  math::Vec3 local_center{.0f};
  math::Vec3 local_max_extents{.0f};
};

struct DirtyMesh {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  entity::EntityId model_entity_id{entity::kInvalidEntityId};
  geometry::MeshHandle mesh_handle{};

  // Retained by the frame packet. Released in FramePacket::Reset().
  resource::MaterialResourceId material_resource_id{};

  math::Vec3 local_center{.0f};
  math::Vec3 local_max_extents{.0f};

  DoubleFrameArray<geometry::Index>* indices{nullptr};
  DoubleFrameArray<geometry::SkinnedVertex>* vertices{nullptr};
};

struct DirtyTransform {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  math::Mat4 transform{1.0f};
};

struct RemovedGeometry {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  entity::EntityId model_entity_id{entity::kInvalidEntityId};
  geometry::MeshHandle mesh_handle{};
};

struct AddedLight {
  rendering::LightHandle light_handle{};
  rendering::LightProperties props{};
  rendering::LightShadow shadow{};
};

struct DirtyLight {
  rendering::LightHandle light_handle{};
  rendering::LightProperties props{};
  rendering::LightShadow shadow{};
};

struct RemovedLight {
  rendering::LightHandle light_handle{};
};

bool operator==(const AddedGeometry& lhs, const AddedGeometry& rhs) noexcept;
bool operator==(const DirtyMesh& lhs, const DirtyMesh& rhs) noexcept;
bool operator==(const DirtyTransform& lhs, const DirtyTransform& rhs) noexcept;
bool operator==(const RemovedGeometry& lhs,
                const RemovedGeometry& rhs) noexcept;
bool operator==(const AddedLight& lhs, const AddedLight& rhs) noexcept;
bool operator==(const DirtyLight& lhs, const DirtyLight& rhs) noexcept;
bool operator==(const RemovedLight& lhs, const RemovedLight& rhs) noexcept;

HashValue GenerateHash(const AddedGeometry& value);
HashValue GenerateHash(const DirtyMesh& value);
HashValue GenerateHash(const DirtyTransform& value);
HashValue GenerateHash(const RemovedGeometry& value);
HashValue GenerateHash(const AddedLight& value);
HashValue GenerateHash(const DirtyLight& value);
HashValue GenerateHash(const RemovedLight& value);

using CameraViews = DoubleFrameArray<rendering::CameraView>;

using AddedGeometries = DoubleFrameOrderedSet<AddedGeometry>;
using DirtyMeshes = DoubleFrameOrderedSet<DirtyMesh>;
using DirtyTransforms = DoubleFrameOrderedSet<DirtyTransform>;
using RemovedGeometries = DoubleFrameOrderedSet<RemovedGeometry>;
using AddedLights = DoubleFrameOrderedSet<AddedLight>;
using DirtyLights = DoubleFrameOrderedSet<DirtyLight>;
using RemovedLights = DoubleFrameOrderedSet<RemovedLight>;
using SkinningBindings = DoubleFrameArray<animation::SkinningBinding>;
using MatrixPalettes = DoubleFrameArray<animation::MatrixPalette>;

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
                               entity::EntityId model_entity_id,
                               geometry::MeshHandle mesh_handle);

  void RegisterNewLight(rendering::LightHandle light_handle,
                        const rendering::LightProperties* props,
                        const rendering::LightShadow* shadow);

  void RegisterDirtyLight(rendering::LightHandle light_handle,
                          const rendering::LightProperties* props,
                          const rendering::LightShadow* shadow);

  void RegisterRemovedLight(rendering::LightHandle light_handle);

  const rendering::CameraView* GetMainCameraView() const;
  const rendering::RenderCameraData* GetMainCameraData() const;
#ifdef COMET_DEBUG
  const rendering::CameraView* GetDebugCameraView() const;
  const rendering::RenderCameraData* GetDebugCameraData() const;
#endif  // COMET_DEBUG

  bool IsFrameStageStarted(FrameStage stage) const;
  bool IsFrameStageFinished(FrameStage stage) const;

  void Reset();

  bool is_populated{false};
  bool can_present{true};
#ifdef COMET_DEBUG
  bool has_debug_camera{false};
#endif  // COMET_DEBUG
  FrameCount frame_count{0};
  f64 lag{.0f};
  f64 time{.0f};
  time::Interpolation interpolation{.0f};
  StageTimes stage_times[kFrameStageCount]{};
  math::Vec3 ambient_color{rendering::kColorWhiteRgb};
  usize draw_count{0};

  usize main_camera_view_index{kInvalidIndex};
  CameraViews* camera_views{nullptr};

  AddedGeometries* added_geometries{nullptr};
  DirtyMeshes* dirty_meshes{nullptr};
  DirtyTransforms* dirty_transforms{nullptr};
  RemovedGeometries* removed_geometries{nullptr};
  AddedLights* added_lights{nullptr};
  DirtyLights* dirty_lights{nullptr};
  RemovedLights* removed_lights{nullptr};
  SkinningBindings* skinning_bindings{nullptr};
  MatrixPalettes* matrix_palettes{nullptr};

  job::Counter* counter{nullptr};
  void* rendering_data{nullptr};

 private:
  fiber::FiberMutex added_geometries_mtx{};
  fiber::FiberMutex dirty_meshes_mtx{};
  fiber::FiberMutex dirty_transforms_mtx{};
  fiber::FiberMutex removed_geometries_mtx{};
  fiber::FiberMutex added_lights_mtx{};
  fiber::FiberMutex dirty_lights_mtx{};
  fiber::FiberMutex removed_lights_mtx{};
};
}  // namespace frame
}  // namespace comet

#endif  // COMET_COMET_RUNTIME_FRAME_FRAME_PACKET_H_