// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "animation_manager.h"

#include "comet/entity/entity_manager.h"
#include "comet/geometry/component/mesh_component.h"

namespace comet {
namespace animation {
AnimationManager& AnimationManager::Get() {
  static AnimationManager singleton{};
  return singleton;
}

void AnimationManager::Update(f64& lag) {
  auto& entity_manager{entity::EntityManager::Get()};

  entity_manager.Each<geometry::MeshComponent, geometry::SkeletonComponent>(
      [&](auto entity_id) {
        auto* mesh_cmp{
            entity_manager.GetComponent<geometry::MeshComponent>(entity_id)};
        auto* skeleton_cmp{
            entity_manager.GetComponent<geometry::SkeletonComponent>(
                entity_id)};

        auto& vertices{mesh_cmp->mesh->vertices};
        auto& resource_vertices{skeleton_cmp->resource->vertices};

        // TODO(m4jr0): Update geometry according to animations.
        mesh_cmp->mesh->is_dirty = true;
      });
}
}  // namespace animation
}  // namespace comet
