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

#include "comet/core/type/array.h"
#include "comet/core/type/ordered_set.h"

namespace comet {
namespace frame {
void FramePacket::RegisterNewGeometry(
    entity::EntityId entity_id, const geometry::MeshComponent* mesh_cmp,
    const physics::TransformComponent* transform_cmp) {
  auto* from_mesh{mesh_cmp->mesh};
  COMET_ASSERT(from_mesh != nullptr, "Mesh from mesh component is null!");

  AddedGeometry geometry{
      .entity_id = entity_id,
      .model_entity_id = mesh_cmp->model_entity_id,
      .mesh_id = from_mesh->id,
      .material_resource = mesh_cmp->material_resource,

      .indices = COMET_DOUBLE_FRAME_ARRAY(geometry::Index),
      .vertices = COMET_DOUBLE_FRAME_ARRAY(geometry::SkinnedVertex),

      .transform = transform_cmp->global,
      .local_center = from_mesh->local_center,
      .local_max_extents = from_mesh->local_max_extents,
  };

  geometry.vertices->PushFromRange(from_mesh->vertices);
  geometry.indices->PushFromRange(from_mesh->indices);

  fiber::FiberLockGuard lock{added_geometries_mtx};
  added_geometries->Add(std::move(geometry));
}

void FramePacket::RegisterDirtyMesh(entity::EntityId entity_id,
                                    const geometry::MeshComponent* mesh_cmp) {
  COMET_ASSERT(mesh_cmp != nullptr, "Mesh component is null!");
  auto* from_mesh{mesh_cmp->mesh};
  COMET_ASSERT(from_mesh != nullptr, "Mesh from mesh component is null!");

  DirtyMesh mesh{
      .entity_id = entity_id,
      .model_entity_id = mesh_cmp->model_entity_id,
      .mesh_id = from_mesh->id,
      .material_resource = mesh_cmp->material_resource,

      .local_center = from_mesh->local_center,
      .local_max_extents = from_mesh->local_max_extents,

      .indices = COMET_DOUBLE_FRAME_ARRAY(geometry::Index),
      .vertices = COMET_DOUBLE_FRAME_ARRAY(geometry::SkinnedVertex),
  };

  mesh.vertices->PushFromRange(from_mesh->vertices);
  mesh.indices->PushFromRange(from_mesh->indices);

  fiber::FiberLockGuard lock{dirty_meshes_mtx};
  dirty_meshes->Add(std::move(mesh));
}

void FramePacket::RegisterDirtyTransform(
    entity::EntityId entity_id,
    const physics::TransformComponent* transform_cmp) {
  COMET_ASSERT(transform_cmp != nullptr, "Transform component is null!");

  DirtyTransform transform{
      .entity_id = entity_id,
      .transform = transform_cmp->global,
  };

  fiber::FiberLockGuard lock{dirty_transforms_mtx};
  dirty_transforms->Add(std::move(transform));
}

void FramePacket::RegisterRemovedGeometry(entity::EntityId entity_id,
                                          entity::EntityId model_entity_id,
                                          geometry::MeshId mesh_id) {
  RemovedGeometry geometry{
      .entity_id = entity_id,
      .model_entity_id = model_entity_id,
      .mesh_id = mesh_id,
  };

  fiber::FiberLockGuard lock{removed_geometries_mtx};
  removed_geometries->Add(std::move(geometry));
}

void FramePacket::RegisterNewLight(rendering::LightId light_id,
                                   const rendering::LightProperties* props,
                                   const rendering::LightShadow* shadow) {
  AddedLight light{
      .light_id = light_id,
      .props = *props,
      .shadow = *shadow,
  };

  fiber::FiberLockGuard lock{added_lights_mtx};
  added_lights->Add(std::move(light));
}

void FramePacket::RegisterDirtyLight(rendering::LightId light_id,
                                     const rendering::LightProperties* props,
                                     const rendering::LightShadow* shadow) {
  DirtyLight light{
      .light_id = light_id,
      .props = *props,
      .shadow = *shadow,
  };

  fiber::FiberLockGuard lock{dirty_lights_mtx};
  dirty_lights->Add(std::move(light));
}

void FramePacket::RegisterRemovedLight(rendering::LightId light_id) {
  RemovedLight light{
      .light_id = light_id,
  };

  fiber::FiberLockGuard lock{removed_lights_mtx};
  removed_lights->Add(std::move(light));
}

bool FramePacket::IsFrameStageStarted(FrameStage stage) const {
  COMET_ASSERT(stage >= 0 && stage < kFrameStageCount,
               "Invalid frame stage: ", stage, "!");
  return stage_times[stage].start > 0;
}

bool FramePacket::IsFrameStageFinished(FrameStage stage) const {
  COMET_ASSERT(stage >= 0 && stage < kFrameStageCount,
               "Invalid frame stage: ", stage, "!");
  return stage_times[stage].end > 0;
}

void FramePacket::FramePacket::Reset() {
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
  return comet::GenerateHash(value.light_id);
}

HashValue GenerateHash(const DirtyLight& value) {
  return comet::GenerateHash(value.light_id);
}

HashValue GenerateHash(const RemovedLight& value) {
  return comet::GenerateHash(value.light_id);
}
}  // namespace frame
}  // namespace comet
