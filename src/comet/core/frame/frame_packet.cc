// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "frame_packet.h"

#include <utility>

#include "comet/core/type/array.h"
#include "comet/core/type/ordered_set.h"

namespace comet {
namespace frame {
void FramePacket::RegisterNewGeometry(
    entity::EntityId entity_id, const geometry::MeshComponent* mesh_cmp,
    const physics::TransformComponent* transform_cmp) {
  auto* from_mesh{mesh_cmp->mesh};
  COMET_ASSERT(from_mesh != nullptr, "Mesh from mesh component is null!");

  AddedGeometry geometry{};
  geometry.entity_id = entity_id;
  geometry.model_entity_id = mesh_cmp->model_entity_id;
  geometry.mesh_id = from_mesh->id;
  geometry.material_resource = mesh_cmp->material_resource;

  geometry.vertices = COMET_DOUBLE_FRAME_ARRAY(geometry::SkinnedVertex);
  geometry.indices = COMET_DOUBLE_FRAME_ARRAY(geometry::Index);
  geometry.vertices->PushFromRange(from_mesh->vertices);
  geometry.indices->PushFromRange(from_mesh->indices);

  geometry.transform = transform_cmp->global;
  geometry.local_center = from_mesh->local_center;
  geometry.local_max_extents = from_mesh->local_max_extents;

  fiber::FiberLockGuard lock{added_geometries_mtx};
  added_geometries->Add(std::move(geometry));
}

void FramePacket::RegisterDirtyMesh(entity::EntityId entity_id,
                                    const geometry::MeshComponent* mesh_cmp) {
  COMET_ASSERT(mesh_cmp != nullptr, "Mesh component is null!");
  auto* from_mesh{mesh_cmp->mesh};
  COMET_ASSERT(from_mesh != nullptr, "Mesh from mesh component is null!");

  DirtyMesh mesh{};
  mesh.entity_id = entity_id;
  mesh.model_entity_id = mesh_cmp->model_entity_id;
  mesh.mesh_id = from_mesh->id;
  mesh.material_resource = mesh_cmp->material_resource;

  mesh.vertices = COMET_DOUBLE_FRAME_ARRAY(geometry::SkinnedVertex);
  mesh.indices = COMET_DOUBLE_FRAME_ARRAY(geometry::Index);

  mesh.vertices->PushFromRange(from_mesh->vertices);
  mesh.indices->PushFromRange(from_mesh->indices);

  mesh.local_center = from_mesh->local_center;
  mesh.local_max_extents = from_mesh->local_max_extents;

  fiber::FiberLockGuard lock{dirty_meshes_mtx};
  dirty_meshes->Add(std::move(mesh));
}

void FramePacket::RegisterDirtyTransform(
    entity::EntityId entity_id,
    const physics::TransformComponent* transform_cmp) {
  COMET_ASSERT(transform_cmp != nullptr, "Transform component is null!");

  DirtyTransform transform{};
  transform.entity_id = entity_id;
  transform.transform = transform_cmp->global;

  fiber::FiberLockGuard lock{dirty_transforms_mtx};
  dirty_transforms->Add(std::move(transform));
}

void FramePacket::RegisterRemovedGeometry(entity::EntityId entity_id,
                                          entity::EntityId model_entity_id,
                                          geometry::MeshId mesh_id) {
  RemovedGeometry geometry{};
  geometry.entity_id = entity_id;
  geometry.model_entity_id = model_entity_id;
  geometry.mesh_id = mesh_id;

  fiber::FiberLockGuard lock{removed_geometries_mtx};
  removed_geometries->Add(std::move(geometry));
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

  is_rendering_skipped = false;
  frame_count = 0;
  lag = .0f;
  time = .0f;
  interpolation = .0f;

  for (usize i{0}; i < kFrameStageCount; ++i) {
    stage_times[i].start = 0;
    stage_times[i].end = 0;
  }

  projection_matrix = math::Mat4{};
  view_matrix = math::Mat4{};
  draw_count = 0;

  added_geometries = COMET_DOUBLE_FRAME_ORDERED_SET(AddedGeometry);
  dirty_meshes = COMET_DOUBLE_FRAME_ORDERED_SET(DirtyMesh);
  dirty_transforms = COMET_DOUBLE_FRAME_ORDERED_SET(DirtyTransform);
  removed_geometries = COMET_DOUBLE_FRAME_ORDERED_SET(RemovedGeometry);
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
}  // namespace frame
}  // namespace comet
