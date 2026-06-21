// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_WRITE_OPERATION_TEMPLATE_H_
#define COMET_RUNTIME_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_WRITE_OPERATION_TEMPLATE_H_

#include "comet/core/essentials.h"
#include "comet/runtime/entity/utils/archetype_utils.h"

namespace comet {
namespace entity {
template <typename... ComponentTypes>
void EntityManager::AddComponents(EntityId entity_id,
                                  const ComponentTypes&... components) {
  COMET_ASSERT(entity_id != kInvalidEntityId, "EntityManager::AddComponents",
               "invalid entity id");

  fiber::FiberLockGuard lock{pending_mutex_};
  auto& entity{GetOrCreatePendingEntity(entity_id)};

  COMET_ASSERT(!entity.is_destroyed, "EntityManager::AddComponents",
               "entity scheduled for destruction", "entity_id", entity_id);

  (DeferAddingComponent(&entity, components), ...);
}

template <typename... ComponentTypes>
void EntityManager::AddChildComponents(EntityId entity_id, EntityId parent_id,
                                       const ComponentTypes&... components) {
  COMET_ASSERT(entity_id != kInvalidEntityId,
               "EntityManager::AddChildComponents", "invalid entity id");
  COMET_ASSERT(parent_id != kInvalidEntityId,
               "EntityManager::AddChildComponents", "invalid parent entity id");

  fiber::FiberLockGuard lock{pending_mutex_};
  auto& entity{GetOrCreatePendingEntity(entity_id)};

  COMET_ASSERT(!entity.is_destroyed, "EntityManager::AddChildComponents",
               "entity scheduled for destruction", "entity_id", entity_id);

  (DeferAddingComponent(&entity, components), ...);
  DeferAddingParent(&entity, parent_id);
}

template <typename... ComponentIds>
void EntityManager::RemoveComponents(EntityId entity_id,
                                     ComponentIds&&... component_ids) {
  COMET_ASSERT(entity_id != kInvalidEntityId, "EntityManager::RemoveComponents",
               "invalid entity id");

  fiber::FiberLockGuard lock{pending_mutex_};
  auto& entity{GetOrCreatePendingEntity(entity_id)};

  COMET_ASSERT(!entity.is_destroyed, "EntityManager::RemoveComponents",
               "entity scheduled for destruction", "entity_id", entity_id);

  (DeferRemovingComponent(&entity, std::forward<ComponentIds>(component_ids)),
   ...);
}

template <typename... ComponentTypes>
void EntityManager::RemoveComponents(EntityId entity_id) {
  COMET_ASSERT(entity_id != kInvalidEntityId, "EntityManager::RemoveComponents",
               "invalid entity id");

  fiber::FiberLockGuard lock{pending_mutex_};
  auto& entity{GetOrCreatePendingEntity(entity_id)};

  COMET_ASSERT(!entity.is_destroyed,
               "EntityManager::RemoveComponents<ComponentTypes...>",
               "entity scheduled for destruction", "entity_id", entity_id);

  (DeferRemovingComponent(&entity,
                          ComponentTypeDescrGetter<ComponentTypes>::Get().id),
   ...);
}

template <typename EntityType>
Archetype* EntityManager::GetOrGenerateArchetype(EntityType&& entity_type) {
  for (auto& archetype : archetypes_) {
    if (archetype->entity_type == entity_type) {
      return archetype.get();
    }
  }

  auto archetype{GenerateArchetype()};
  archetype->entity_type = std::forward<EntityType>(entity_type);

  ArchetypeId archetype_id{0};

  for (const auto component_type_id : archetype->entity_type) {
    archetype_id = HashCombine(archetype_id, component_type_id);
    archetype->components.EmplaceLast(ComponentArray{nullptr, 0});
  }

  archetype->id = archetype_id;
  usize i{0};

  for (const auto component_type_id : archetype->entity_type) {
    COMET_ASSERT(known_component_types_.IsContained(component_type_id),
                 "EntityManager::GetOrGenerateArchetype",
                 "component type is not known", "component_type_id",
                 component_type_id);

    auto& registered_component_type{
        known_component_types_.Get(component_type_id)};

    registered_component_type.archetype_map.GetOrAdd(archetype->id)
        .cmp_array_index = i++;

    ++registered_component_type.archetype_ref_count;
  }

  auto* archetype_p{archetype.get()};
  archetype_p->index = archetypes_.GetSize();
  archetypes_.PushLast(std::move(archetype));
  return archetype_p;
}
}  // namespace entity
}  // namespace comet

#endif  // COMET_RUNTIME_ENTITY_MANAGER_IMPL_ENTITY_MANAGER_WRITE_OPERATION_TEMPLATE_H_