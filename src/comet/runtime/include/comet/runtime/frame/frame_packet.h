// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RUNTIME_FRAME_FRAME_PACKET_H_
#define COMET_COMET_RUNTIME_FRAME_FRAME_PACKET_H_

#include "comet/runtime/animation/animation_skinning.h"
#include "comet/core/fiber/fiber_primitive.h"
#include "comet/core/job/job.h"
#include "comet/core/essentials.h"
#include "comet/runtime/frame/frame_container.h"
#include "comet/core/hash/hash.h"
#include "comet/runtime/light/light_handle.h"
#include "comet/runtime/geometry/mesh.h"
#include "comet/runtime/entity/entity_id.h"
#include "comet/runtime/geometry/component/mesh_component.h"
#include "comet/data/geometry/mesh.h"
#include "comet/core/math/matrix.h"
#include "comet/core/math/vector.h"
#include "comet/runtime/transform/component/transform_component.h"
#include "comet/runtime/camera/camera.h"
#include "comet/data/light/light.h"
#include "comet/data/render/texture.h"
#include "comet/data/resource/material/material_resource.h"
#include "comet/runtime/time/time_manager.h"

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
  light::LightHandle light_handle{};
  light::LightProperties props{};
  light::LightShadow shadow{};
};

struct DirtyLight {
  light::LightHandle light_handle{};
  light::LightProperties props{};
  light::LightShadow shadow{};
};

struct RemovedLight {
  light::LightHandle light_handle{};
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

using CameraViews = DoubleFrameArray<camera::CameraView>;

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

  void RegisterNewLight(light::LightHandle light_handle,
                        const light::LightProperties* props,
                        const light::LightShadow* shadow);

  void RegisterDirtyLight(light::LightHandle light_handle,
                          const light::LightProperties* props,
                          const light::LightShadow* shadow);

  void RegisterRemovedLight(light::LightHandle light_handle);

  const camera::CameraView* GetMainCameraView() const;
  const camera::CameraViewData* GetMainCameraData() const;
#ifdef COMET_DEBUG
  const camera::CameraView* GetDebugCameraView() const;
  const camera::CameraViewData* GetDebugCameraData() const;
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
  math::Vec3 ambient_color{render::kColorWhiteRgb};
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