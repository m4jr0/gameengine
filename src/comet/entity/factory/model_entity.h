// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_FACTORY_MODEL_ENTITY_H_
#define COMET_COMET_ENTITY_FACTORY_MODEL_ENTITY_H_

#include "comet_precompile.h"

#include "comet/core/engine.h"
#include "comet/entity/component/mesh_component.h"
#include "comet/entity/component/transform_component.h"
#include "comet/entity/entity.h"
#include "comet/resource/model_resource.h"

namespace comet {
namespace entity {
template <typename ResourcePath>
EntityId CreateModelEntity(ResourcePath&& model_path) {
  auto& resource_manager{Engine::Get().GetResourceManager()};

  const auto* model{resource_manager.Load<resource::model::ModelResource>(
      std::forward<ResourcePath>(model_path))};

  if (model == nullptr) {
    return kInvalidEntityId;
  }

  auto& entity_manager{Engine::Get().GetEntityManager()};

  auto root_entity_id{entity_manager.CreateEntity(TransformComponent{})};

  EntityId first_entity{kInvalidEntityId};
  EntityId prev_entity{kInvalidEntityId};

  for (const auto& mesh : model->meshes) {
    auto child_entity_id{entity_manager.CreateEntity(
        MeshComponent{&mesh, mesh.textures.size()},
        TransformComponent{first_entity, prev_entity, kInvalidEntityId,
                           root_entity_id})};

    auto* mesh_cmp{entity_manager.GetComponent<MeshComponent>(child_entity_id)};

    COMET_ASSERT(mesh_cmp->texture_count <= kTextureCount,
                 "Too many textures on mesh! ", mesh_cmp->texture_count, " > ",
                 kTextureCount, ".");

    const auto max{mesh_cmp->texture_count};

    for (uindex i{0}; i < max; ++i) {
      mesh_cmp->textures[i] =
          resource_manager
              .LoadFromResourceId<resource::texture::TextureResource>(
                  mesh.textures[i].texture_id);
    }

    // Case: first child entity added.
    if (first_entity == kInvalidEntityId) {
      first_entity = child_entity_id;
    } else {
      entity_manager.GetComponent<TransformComponent>(prev_entity)->next =
          child_entity_id;
    }

    prev_entity = child_entity_id;
  }

  return root_entity_id;
}
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_FACTORY_MODEL_ENTITY_H_