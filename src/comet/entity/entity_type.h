// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_TYPE_H_
#define COMET_COMET_ENTITY_ENTITY_TYPE_H_

#include "comet_precompile.h"

#include "comet/entity/component.h"
#include "comet/entity/entity_id.h"

namespace comet {
namespace entity {
using EntityType = std::vector<EntityId>;

EntityType GenerateEntityType(
    const std::vector<ComponentDescr>& component_descrs);
EntityType GenerateEntityType(std::vector<EntityId> component_type_ids);
EntityType AddToEntityType(const EntityType& entity_type,
                           const EntityType& to_add);
EntityType RemoveFromEntityType(const EntityType& from_entity_type,
                                const EntityType& to_remove);
EntityType& CleanEntityType(EntityType& entity_type);
}  // namespace entity
}  // namespace comet
#endif  // COMET_COMET_ENTITY_ENTITY_TYPE_H_
