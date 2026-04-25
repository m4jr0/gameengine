// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "entity_primitive_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_manager.h"
#include "comet/entity/entity_manager.h"
#include "comet/geometry/component/mesh_component.h"
#include "comet/geometry/geometry_manager.h"
#include "comet/physics/physics_manager.h"

namespace comet {
namespace entity {
EntityId PrimitiveHandler::GenerateCube(f32 size,
                                        resource::ResourceLifeSpan) const {
  auto& entity_manager{EntityManager::Get()};
  auto& geometry_manager{geometry::GeometryManager::Get()};
  auto& physics_manager{physics::PhysicsManager::Get()};

  const auto entity_id{entity_manager.Generate()};
  const auto mesh_handle{geometry_manager.GenerateCube(size)};

  geometry::MeshComponent mesh_cmp{};
  mesh_cmp.entity_id = entity_id;
  mesh_cmp.model_entity_id = entity_id;
  mesh_cmp.mesh_handle = mesh_handle;
  mesh_cmp.material_resource_id = resource::GetDefaultMaterialId();

  const auto transform_root_cmp{
      physics_manager.GenerateTransformRootComponent()};
  const auto transform_cmp{
      physics_manager.GenerateTransformComponent(entity_id)};

  entity_manager.AddComponents(entity_id, mesh_cmp, transform_cmp,
                               transform_root_cmp);

  frame::FrameManager::Get().GetLogicFramePacket()->RegisterNewGeometry(
      entity_id, &mesh_cmp, &transform_cmp);

  return entity_id;
}
}  // namespace entity
}  // namespace comet