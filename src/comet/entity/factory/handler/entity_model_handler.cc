// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entity_model_handler.h"

#include "comet/core/type/stl_types.h"
#include "comet/core/type/tstring.h"
#include "comet/entity/entity_component.h"
#include "comet/physics/component/transform_component.h"
#include "comet/resource/component/mesh_component.h"
#include "comet/resource/model_resource.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace entity {
EntityId ModelHandler::Generate(CTStringView model_path) const {
  const auto* model{
      resource::ResourceManager::Get().Load<resource::ModelResource>(
          model_path)};

  if (model == nullptr) {
    return kInvalidEntityId;
  }

  auto root_entity_id{GetEntityComponentGenerator<one_frame_vector>()
                          .AddComponent(physics::TransformComponent{})
                          .AddComponent(physics::TransformRootComponent{})
                          .Submit()};

  auto* root_transform_cmp{
      EntityManager::Get().GetComponent<physics::TransformComponent>(
          root_entity_id)};

  root_transform_cmp->parent_entity_id = root_transform_cmp->root_entity_id =
      root_entity_id;

  one_frame_unordered_map<resource::ResourceId, EntityId> entity_ids{};

  // TODO(m4jr0): Optimize code to do one big submit at once.
  for (const auto& mesh : model->meshes) {
    resource::MeshComponent mesh_cmp{};
    mesh_cmp.mesh = &mesh;
    mesh_cmp.material =
        resource::ResourceManager::Get().Load<resource::MaterialResource>(
            mesh.material_id);

    physics::TransformComponent transform_cmp{};
    transform_cmp.root_entity_id = root_entity_id;
    transform_cmp.parent_entity_id =
        mesh.parent_id == resource::kInvalidResourceId
            ? transform_cmp.root_entity_id
            : entity_ids.at(mesh.parent_id);
    transform_cmp.local = mesh.transform;

    entity_ids[mesh.internal_id] =
        GetEntityComponentGenerator<one_frame_vector>()
            .AddComponent(mesh_cmp)
            .AddComponent(transform_cmp)
            .AddParent(transform_cmp.parent_entity_id)
            .Submit();
  }

  return root_entity_id;
}
}  // namespace entity
}  // namespace comet