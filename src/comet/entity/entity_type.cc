// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entity_type.h"

namespace comet {
namespace entity {
EntityType GenerateEntityType(
    const std::vector<ComponentDescr>& component_descrs) {
  EntityType entity_type{};
  entity_type.reserve(component_descrs.size());

  for (const auto& descr : component_descrs) {
    entity_type.push_back(descr.type_descr.id);
  }

  CleanEntityType(entity_type);
  return entity_type;
}

EntityType GenerateEntityType(std::vector<EntityId> component_type_ids) {
  EntityType entity_type{std::move(component_type_ids)};
  CleanEntityType(entity_type);
  return entity_type;
}

EntityType AddToEntityType(const EntityType& entity_type,
                           const EntityType& to_add) {
  EntityType new_type{};
  new_type.reserve(entity_type.size() + to_add.size());

  for (uindex i{0}; i < entity_type.size(); ++i) {
    new_type.push_back(entity_type[i]);
  }

  for (uindex i{0}; i < new_type.capacity() - entity_type.size(); ++i) {
    new_type.push_back(to_add[i]);
  }

  CleanEntityType(new_type);
  return new_type;
}

EntityType RemoveFromEntityType(const EntityType& from_entity_type,
                                const EntityType& to_remove) {
  const auto from_size{from_entity_type.size()};
  const auto to_remove_size{to_remove.size()};

  COMET_ASSERT(from_size >= to_remove_size,
               "Tried to remove too many components from entity type!");

  EntityType entity_type{};
  uindex size{from_size - to_remove_size};
  entity_type.resize(size);

  uindex i{0};
  uindex j{0};
  uindex cursor{0};

  while (i < size) {
    // Entity types are sorted.
    while (j < to_remove_size && to_remove[j] <= from_entity_type[cursor]) {
      ++j;
      ++cursor;
    }

    entity_type[i++] = from_entity_type[cursor++];
  }

  return entity_type;  // Entity type is already sorted.
}

EntityType& CleanEntityType(EntityType& entity_type) {
  std::sort(entity_type.begin(), entity_type.end());
  return entity_type;
}
}  // namespace entity
}  // namespace comet
