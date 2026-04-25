// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_READ_OPERATION_TEMPLATE_H_
#define COMET_COMET_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_READ_OPERATION_TEMPLATE_H_

#include "comet/core/essentials.h"

namespace comet {
namespace entity {
template <typename ComponentType>
bool EntityManager::HasComponent(EntityId entity_id) const {
  return ReadSnapshot("EntityManager::HasComponent", [&] {
    return HasComponentUnlocked<ComponentType>(entity_id);
  });
}

template <typename ComponentType>
ComponentType* EntityManager::GetComponent(EntityId entity_id) {
  return ReadSnapshot("EntityManager::GetComponent", [&] {
    return GetComponentUnlocked<ComponentType>(entity_id);
  });
}

template <typename ComponentType>
const ComponentType* EntityManager::GetComponent(EntityId entity_id) const {
  return ReadSnapshot("EntityManager::GetComponent", [&] {
    return GetComponentUnlocked<ComponentType>(entity_id);
  });
}

template <typename ComponentType>
bool EntityManager::HasComponentUnlocked(EntityId entity_id) const {
  const auto component_type_id{
      ComponentTypeDescrGetter<ComponentType>::Get().id};
  return HasComponentUnlocked(entity_id, component_type_id);
}

template <typename ComponentType>
ComponentType* EntityManager::GetComponentUnlocked(EntityId entity_id) {
  const auto component_type_id{
      ComponentTypeDescrGetter<ComponentType>::Get().id};

  COMET_ASSERT(IsEntityUnlocked(entity_id),
               "EntityManager::GetComponentUnlocked", "entity does not exist",
               "entity_id", entity_id, "component_type_id", component_type_id);

  if (!HasComponentUnlocked(entity_id, component_type_id)) {
    return nullptr;
  }

  const auto& record{records_.Get(entity_id)};
  const auto& archetype_record{known_component_types_.Get(component_type_id)
                                   .archetype_map.Get(record.archetype->id)};

  return reinterpret_cast<ComponentType*>(
             record.archetype->components[archetype_record.cmp_array_index]
                 .elements) +
         record.row;
}

template <typename ComponentType>
const ComponentType* EntityManager::GetComponentUnlocked(
    EntityId entity_id) const {
  const auto component_type_id{
      ComponentTypeDescrGetter<ComponentType>::Get().id};

  COMET_ASSERT(IsEntityUnlocked(entity_id),
               "EntityManager::GetComponentUnlocked", "entity does not exist",
               "entity_id", entity_id, "component_type_id", component_type_id);

  if (!HasComponentUnlocked(entity_id, component_type_id)) {
    return nullptr;
  }

  const auto& record{records_.Get(entity_id)};
  const auto& archetype_record{known_component_types_.Get(component_type_id)
                                   .archetype_map.Get(record.archetype->id)};

  return reinterpret_cast<const ComponentType*>(
             record.archetype->components[archetype_record.cmp_array_index]
                 .elements) +
         record.row;
}

template <typename ComponentType>
ComponentType* EntityManager::GetArchetypeComponentData(Archetype& archetype) {
  const auto component_type_id{
      ComponentTypeDescrGetter<ComponentType>::Get().id};

  const auto cmp_array_index{known_component_types_.Get(component_type_id)
                                 .archetype_map.Get(archetype.id)
                                 .cmp_array_index};

  return reinterpret_cast<ComponentType*>(
      archetype.components[cmp_array_index].elements);
}

template <typename ComponentType>
const ComponentType* EntityManager::GetArchetypeComponentData(
    const Archetype& archetype) const {
  const auto component_type_id{
      ComponentTypeDescrGetter<ComponentType>::Get().id};

  const auto cmp_array_index{known_component_types_.Get(component_type_id)
                                 .archetype_map.Get(archetype.id)
                                 .cmp_array_index};

  return reinterpret_cast<const ComponentType*>(
      archetype.components[cmp_array_index].elements);
}

template <usize N>
bool EntityManager::DoesArchetypeMatch(
    const Archetype& archetype, const StaticArray<EntityId, N>& all_ids) {
  if (all_ids.GetSize() == 0) {
    return true;
  }

  if (archetype.entity_type.GetSize() < all_ids.GetSize()) {
    return false;
  }

  usize count{0};

  for (const auto component_type_id : archetype.entity_type) {
    if (component_type_id == all_ids[count]) {
      ++count;
    }

    if (count == all_ids.GetSize()) {
      return true;
    }
  }

  return false;
}
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_READ_OPERATION_TEMPLATE_H_