// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entity_type.h"

#include <algorithm>

#include "comet/entity/entity_memory_manager.h"

namespace comet {
namespace entity {
EntityType GenerateEntityType(const Array<ComponentDescr>& component_descrs) {
  EntityType entity_type{&EntityMemoryManager::Get().GetEntityTypeAllocator()};
  entity_type.Reserve(component_descrs.GetSize());

  for (const auto& descr : component_descrs) {
    entity_type.PushBack(descr.type_descr.id);
  }

  CleanEntityType(entity_type);
  return entity_type;
}

EntityType GenerateEntityType(Array<EntityId> component_type_ids) {
  EntityType entity_type{std::move(component_type_ids)};
  CleanEntityType(entity_type);
  return entity_type;
}

EntityType AddToEntityType(const EntityType& entity_type,
                           const EntityType& to_add) {
  EntityType new_type{&EntityMemoryManager::Get().GetEntityTypeAllocator()};
  new_type.Reserve(entity_type.GetSize() + to_add.GetSize());

  for (usize i{0}; i < entity_type.GetSize(); ++i) {
    new_type.PushBack(entity_type[i]);
  }

  for (usize i{0}; i < new_type.GetCapacity() - entity_type.GetSize(); ++i) {
    new_type.PushBack(to_add[i]);
  }

  CleanEntityType(new_type);
  return new_type;
}

EntityType RemoveFromEntityType(const EntityType& from_entity_type,
                                const EntityType& to_remove) {
  const auto from_size{from_entity_type.GetSize()};
  const auto to_remove_size{to_remove.GetSize()};

  COMET_ASSERT(from_size >= to_remove_size,
               "Tried to remove too many components from entity type!");

  EntityType entity_type{&EntityMemoryManager::Get().GetEntityTypeAllocator()};
  usize size{from_size - to_remove_size};
  entity_type.Resize(size);

  usize i{0};
  usize j{0};
  usize cursor{0};

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
