// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "frame_packet.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type/ordered_set.h"
#include "comet/geometry/geometry_manager.h"

namespace comet {
namespace frame {
void FramePacket::RegisterNewGeometry(
    entity::EntityId entity_id, const geometry::MeshComponent* mesh_cmp,
    const physics::TransformComponent* transform_cmp) {
  COMET_ASSERT(mesh_cmp != nullptr, "FramePacket::RegisterNewGeometry",
               "mesh component is null");
  COMET_ASSERT(transform_cmp != nullptr, "FramePacket::RegisterNewGeometry",
               "transform component is null");
  COMET_ASSERT(mesh_cmp->mesh_handle, "FramePacket::RegisterNewGeometry",
               "mesh handle is invalid");

  AddedGeometry geometry{
      .entity_id = entity_id,
      .model_entity_id = mesh_cmp->model_entity_id,
      .material_resource_id = mesh_cmp->material_resource_id,
      .indices = COMET_DOUBLE_FRAME_ARRAY(geometry::Index),
      .vertices = COMET_DOUBLE_FRAME_ARRAY(geometry::SkinnedVertex),
      .transform = transform_cmp->global,
  };

  geometry::GeometryManager::Get().PopulateGeometryData(mesh_cmp->mesh_handle,
                                                        geometry);

  fiber::FiberLockGuard lock{added_geometries_mtx};
  added_geometries->Add(std::move(geometry));
}

void FramePacket::RegisterDirtyMesh(entity::EntityId entity_id,
                                    const geometry::MeshComponent* mesh_cmp) {
  COMET_ASSERT(mesh_cmp != nullptr, "FramePacket::RegisterDirtyMesh",
               "mesh component is null");
  COMET_ASSERT(mesh_cmp->mesh_handle, "FramePacket::RegisterDirtyMesh",
               "mesh handle is invalid");

  DirtyMesh mesh{
      .entity_id = entity_id,
      .model_entity_id = mesh_cmp->model_entity_id,
      .material_resource_id = mesh_cmp->material_resource_id,
      .indices = COMET_DOUBLE_FRAME_ARRAY(geometry::Index),
      .vertices = COMET_DOUBLE_FRAME_ARRAY(geometry::SkinnedVertex),
  };

  geometry::GeometryManager::Get().PopulateGeometryData(mesh_cmp->mesh_handle,
                                                        mesh);

  fiber::FiberLockGuard lock{dirty_meshes_mtx};
  dirty_meshes->Add(std::move(mesh));
}

void FramePacket::RegisterDirtyTransform(
    entity::EntityId entity_id,
    const physics::TransformComponent* transform_cmp) {
  COMET_ASSERT(transform_cmp != nullptr, "FramePacket::RegisterDirtyTransform",
               "transform component is null");

  DirtyTransform transform{
      .entity_id = entity_id,
      .transform = transform_cmp->global,
  };

  fiber::FiberLockGuard lock{dirty_transforms_mtx};
  dirty_transforms->Add(std::move(transform));
}

void FramePacket::RegisterRemovedGeometry(entity::EntityId entity_id,
                                          entity::EntityId model_entity_id,
                                          geometry::MeshHandle mesh_handle) {
  RemovedGeometry geometry{
      .entity_id = entity_id,
      .model_entity_id = model_entity_id,
      .mesh_handle = mesh_handle,
  };

  fiber::FiberLockGuard lock{removed_geometries_mtx};
  removed_geometries->Add(std::move(geometry));
}

void FramePacket::RegisterNewLight(rendering::LightHandle light_handle,
                                   const rendering::LightProperties* props,
                                   const rendering::LightShadow* shadow) {
  COMET_ASSERT(props != nullptr, "FramePacket::RegisterNewLight",
               "light properties are null");
  COMET_ASSERT(shadow != nullptr, "FramePacket::RegisterNewLight",
               "light shadow is null");

  AddedLight light{
      .light_handle = light_handle,
      .props = *props,
      .shadow = *shadow,
  };

  fiber::FiberLockGuard lock{added_lights_mtx};
  added_lights->Add(std::move(light));
}

void FramePacket::RegisterDirtyLight(rendering::LightHandle light_handle,
                                     const rendering::LightProperties* props,
                                     const rendering::LightShadow* shadow) {
  COMET_ASSERT(props != nullptr, "FramePacket::RegisterDirtyLight",
               "light properties are null");
  COMET_ASSERT(shadow != nullptr, "FramePacket::RegisterDirtyLight",
               "light shadow is null");

  DirtyLight light{
      .light_handle = light_handle,
      .props = *props,
      .shadow = *shadow,
  };

  fiber::FiberLockGuard lock{dirty_lights_mtx};
  dirty_lights->Add(std::move(light));
}

void FramePacket::RegisterRemovedLight(rendering::LightHandle light_handle) {
  RemovedLight light{
      .light_handle = light_handle,
  };

  fiber::FiberLockGuard lock{removed_lights_mtx};
  removed_lights->Add(std::move(light));
}

bool FramePacket::IsFrameStageStarted(FrameStage stage) const {
  COMET_ASSERT(stage >= 0 && stage < kFrameStageCount,
               "FramePacket::IsFrameStageStarted", "frame stage is invalid",
               "stage", stage);
  return stage_times[stage].start > 0;
}

bool FramePacket::IsFrameStageFinished(FrameStage stage) const {
  COMET_ASSERT(stage >= 0 && stage < kFrameStageCount,
               "FramePacket::IsFrameStageFinished", "frame stage is invalid",
               "stage", stage);
  return stage_times[stage].end > 0;
}

void FramePacket::Reset() {
  // No locking required. This function is designed for single-threaded
  // execution.
  can_present = true;
  frame_count = 0;
  lag = .0f;
  time = .0f;
  interpolation = .0f;

  for (usize i{0}; i < kFrameStageCount; ++i) {
    stage_times[i].start = 0;
    stage_times[i].end = 0;
  }

  ambient_color = rendering::kColorWhiteRgb;
  draw_count = 0;
  camera_data = {};

  added_geometries = COMET_DOUBLE_FRAME_ORDERED_SET(AddedGeometry);
  dirty_meshes = COMET_DOUBLE_FRAME_ORDERED_SET(DirtyMesh);
  dirty_transforms = COMET_DOUBLE_FRAME_ORDERED_SET(DirtyTransform);
  removed_geometries = COMET_DOUBLE_FRAME_ORDERED_SET(RemovedGeometry);
  added_lights = COMET_DOUBLE_FRAME_ORDERED_SET(AddedLight);
  dirty_lights = COMET_DOUBLE_FRAME_ORDERED_SET(DirtyLight);
  removed_lights = COMET_DOUBLE_FRAME_ORDERED_SET(RemovedLight);
  skinning_bindings = COMET_DOUBLE_FRAME_ARRAY(animation::SkinningBinding);
  matrix_palettes = COMET_DOUBLE_FRAME_ARRAY(animation::MatrixPalette);

  counter = nullptr;
  rendering_data = nullptr;
}

bool operator==(const AddedGeometry& lhs, const AddedGeometry& rhs) noexcept {
  return lhs.entity_id == rhs.entity_id;
}

bool operator==(const DirtyMesh& lhs, const DirtyMesh& rhs) noexcept {
  return lhs.entity_id == rhs.entity_id;
}

bool operator==(const DirtyTransform& lhs, const DirtyTransform& rhs) noexcept {
  return lhs.entity_id == rhs.entity_id;
}

bool operator==(const RemovedGeometry& lhs,
                const RemovedGeometry& rhs) noexcept {
  return lhs.entity_id == rhs.entity_id;
}

bool operator==(const AddedLight& lhs, const AddedLight& rhs) noexcept {
  return lhs.light_handle == rhs.light_handle;
}

bool operator==(const DirtyLight& lhs, const DirtyLight& rhs) noexcept {
  return lhs.light_handle == rhs.light_handle;
}

bool operator==(const RemovedLight& lhs, const RemovedLight& rhs) noexcept {
  return lhs.light_handle == rhs.light_handle;
}

HashValue GenerateHash(const AddedGeometry& value) {
  return comet::GenerateHash(value.entity_id);
}

HashValue GenerateHash(const DirtyMesh& value) {
  return comet::GenerateHash(value.entity_id);
}

HashValue GenerateHash(const DirtyTransform& value) {
  return comet::GenerateHash(value.entity_id);
}

HashValue GenerateHash(const RemovedGeometry& value) {
  return comet::GenerateHash(value.entity_id);
}

HashValue GenerateHash(const AddedLight& value) {
  return comet::GenerateHash(value.light_handle);
}

HashValue GenerateHash(const DirtyLight& value) {
  return comet::GenerateHash(value.light_handle);
}

HashValue GenerateHash(const RemovedLight& value) {
  return comet::GenerateHash(value.light_handle);
}
}  // namespace frame
}  // namespace comet