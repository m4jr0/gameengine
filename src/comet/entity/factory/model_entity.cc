// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model_entity.h"

#include "comet/core/engine.h"
#include "comet/entity/component/mesh_component.h"
#include "comet/entity/component/transform_component.h"
#include "comet/resource/model_resource.h"

namespace comet {
namespace entity {
EntityId GenerateModelEntity(const std::string& model_path) {
  return GenerateModelEntity(model_path.c_str());
}

EntityId GenerateModelEntity(const schar* model_path) {
  auto& resource_manager{Engine::Get().GetResourceManager()};
  const auto* model{resource_manager.Load<resource::ModelResource>(model_path)};

  if (model == nullptr) {
    return kInvalidEntityId;
  }

  auto& entity_manager{Engine::Get().GetEntityManager()};
  auto root_entity_id{entity_manager.Generate(TransformComponent{})};

  EntityId first_entity{kInvalidEntityId};
  EntityId prev_entity{kInvalidEntityId};

  for (const auto& mesh : model->meshes) {
    const auto child_entity_id{entity_manager.Generate(
        MeshComponent{&mesh, resource_manager.Load<resource::MaterialResource>(
                                 mesh.material_id)},
        TransformComponent{first_entity, prev_entity, kInvalidEntityId,
                           root_entity_id})};

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