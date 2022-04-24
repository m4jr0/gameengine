// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_MISC_TYPES_H_
#define COMET_COMET_ENTITY_ENTITY_MISC_TYPES_H_

#include "comet/entity/component/component.h"

namespace comet {
namespace entity {
using EntityType = std::vector<ComponentTypeId>;

struct ComponentDescr {
  uindex size{0};
  uindex alignment{0};
};

struct ComponentArray {
  u8* elements{nullptr};
  u8* first{nullptr};
  uindex size{0};
};

struct Archetype {
  EntityType entity_type;
  std::vector<EntityId> entity_ids;
  std::vector<ComponentArray> components;
};

struct Record {
  Archetype* archetype{nullptr};
  uindex row{kInvalidIndex};
};

EntityType& CleanEntityType(EntityType& entity_type);

template <typename... ComponentTypes>
EntityType GenerateEntityType() {
  std::vector<ComponentTypeId> removed_component_type_ids{
      {ComponentTypes::kComponentTypeId...}};
  EntityType entity_type{{ComponentTypes::kComponentTypeId...}};
  CleanEntityType(entity_type);
  return entity_type;
}

template <typename AddedEntityType>
EntityType AddToEntityType(EntityType from_entity_type,
                           AddedEntityType&& to_add) {
  auto entity_type(std::move(from_entity_type));
  const auto old_size{entity_type.size()};
  const auto to_add_size{to_add.size()};
  entity_type.resize(old_size + to_add_size);

  for (uindex i{0}; i < to_add_size; ++i) {
    entity_type[old_size + i] = to_add[i];
  }

  CleanEntityType(entity_type);
  return entity_type;
}

template <typename... ComponentTypes>
EntityType AddToEntityType(EntityType from_entity_type) {
  return AddToEntityType(std::move(from_entity_type),
                         GenerateEntityType<ComponentTypes...>());
}

template <typename RemovedEntityType>
EntityType RemoveFromEntityType(const EntityType& from_entity_type,
                                RemovedEntityType&& to_remove) {
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

template <typename... ComponentTypes>
EntityType RemoveFromEntityType(const EntityType& from_entity_type) {
  return RemoveFromEntityType(from_entity_type,
                              GenerateEntityType<ComponentTypes...>());
}
}  // namespace entity
}  // namespace comet
#endif  // COMET_COMET_ENTITY_ENTITY_MISC_TYPES_H_
