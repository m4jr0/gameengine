// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_MANAGER_ITERATION_TEMPLATE_H_
#define COMET_COMET_ENTITY_ENTITY_MANAGER_ITERATION_TEMPLATE_H_

#include "comet/core/essentials.h"

namespace comet {
namespace entity {
// Public API.
template <typename... ComponentTypes, typename Fn, typename... ComponentTypeIds>
void EntityManager::ForEach(const Fn& fn,
                            ComponentTypeIds... component_type_ids) {
  ReadSnapshot("EntityManager::ForEach", [&] {
    ForEachUnlocked<ComponentTypes...>(fn, component_type_ids...);
  });
}

template <typename... ComponentTypes, typename Fn, typename... ComponentTypeIds>
void EntityManager::ForEach(const Fn& fn,
                            ComponentTypeIds... component_type_ids) const {
  ReadSnapshot("EntityManager::ForEach", [&] {
    ForEachUnlocked<ComponentTypes...>(fn, component_type_ids...);
  });
}

template <typename... ComponentTypes, typename Fn, typename... ComponentTypeIds>
void EntityManager::ForEachId(const Fn& fn,
                              ComponentTypeIds... component_type_ids) const {
  ReadSnapshot("EntityManager::ForEachId", [&] {
    ForEachIdUnlocked<ComponentTypes...>(fn, component_type_ids...);
  });
}

template <typename... ComponentTypes, typename Fn>
void EntityManager::ForEachChild(const Fn& fn, EntityId parent_id) {
  ReadSnapshot("EntityManager::ForEachChild",
               [&] { ForEachChildUnlocked<ComponentTypes...>(fn, parent_id); });
}

template <typename... ComponentTypes, typename Fn>
void EntityManager::ForEachChild(const Fn& fn, EntityId parent_id) const {
  ReadSnapshot("EntityManager::ForEachChild",
               [&] { ForEachChildUnlocked<ComponentTypes...>(fn, parent_id); });
}

template <typename... ComponentTypes, typename Fn>
void EntityManager::ForEachChildId(const Fn& fn, EntityId parent_id) const {
  ReadSnapshot("EntityManager::ForEachChildId", [&] {
    ForEachChildIdUnlocked<ComponentTypes...>(fn, parent_id);
  });
}

// Unprotected API.
template <typename... ComponentTypes, typename Fn, typename... ComponentTypeIds>
void EntityManager::ForEachUnlocked(const Fn& fn,
                                    ComponentTypeIds... component_type_ids) {
  ForEachArchetypeMatching<ComponentTypes...>(
      [&](Archetype& archetype) {
        auto component_arrays{std::tuple<ComponentTypes*...>{
            GetArchetypeComponentData<ComponentTypes>(archetype)...}};

        for (usize entity_index{0}; entity_index < archetype.size;
             ++entity_index) {
          fn(archetype.entity_ids[entity_index],
             (std::get<ComponentTypes*>(component_arrays)[entity_index])...);
        }
      },
      component_type_ids...);
}

template <typename... ComponentTypes, typename Fn, typename... ComponentTypeIds>
void EntityManager::ForEachUnlocked(
    const Fn& fn, ComponentTypeIds... component_type_ids) const {
  ForEachArchetypeMatching<ComponentTypes...>(
      [&](const Archetype& archetype) {
        auto component_arrays{std::tuple<const ComponentTypes*...>{
            GetArchetypeComponentData<ComponentTypes>(archetype)...}};

        for (usize entity_index{0}; entity_index < archetype.size;
             ++entity_index) {
          fn(archetype.entity_ids[entity_index],
             (std::get<const ComponentTypes*>(
                 component_arrays)[entity_index])...);
        }
      },
      component_type_ids...);
}

template <typename... ComponentTypes, typename Fn, typename... ComponentTypeIds>
void EntityManager::ForEachIdUnlocked(
    const Fn& fn, ComponentTypeIds... component_type_ids) const {
  if constexpr (sizeof...(ComponentTypes) == 0 &&
                sizeof...(ComponentTypeIds) == 0) {
    for (const auto& archetype : archetypes_) {
      ForEachEntityIdInArchetype(*archetype, fn);
    }
  } else {
    ForEachArchetypeMatching<ComponentTypes...>(
        [&](const Archetype& archetype) {
          ForEachEntityIdInArchetype(archetype, fn);
        },
        component_type_ids...);
  }
}

template <typename... ComponentTypes, typename Fn>
void EntityManager::ForEachChildUnlocked(const Fn& fn, EntityId parent_id) {
  ForEachUnlocked<ComponentTypes...>(fn, Tag(EntityIdTag::Child, parent_id));
}

template <typename... ComponentTypes, typename Fn>
void EntityManager::ForEachChildUnlocked(const Fn& fn,
                                         EntityId parent_id) const {
  ForEachUnlocked<ComponentTypes...>(fn, Tag(EntityIdTag::Child, parent_id));
}

template <typename... ComponentTypes, typename Fn>
void EntityManager::ForEachChildIdUnlocked(const Fn& fn,
                                           EntityId parent_id) const {
  ForEachIdUnlocked<ComponentTypes...>(fn, Tag(EntityIdTag::Child, parent_id));
}

// Internal helpers.
template <typename Fn>
void EntityManager::ForEachEntityIdInArchetype(const Archetype& archetype,
                                               const Fn& fn) {
  for (usize entity_index{0}; entity_index < archetype.size; ++entity_index) {
    fn(archetype.entity_ids[entity_index]);
  }
}

template <typename... ComponentTypes, typename Fn, typename... ComponentTypeIds>
void EntityManager::ForEachArchetypeMatching(
    const Fn& fn, ComponentTypeIds... component_type_ids) {
  const auto all_ids{
      BuildSortedComponentIdArray<ComponentTypes...>(component_type_ids...)};

  for (auto& archetype : archetypes_) {
    if (!DoesArchetypeMatch(*archetype, all_ids)) {
      continue;
    }

    fn(*archetype);
  }
}

template <typename... ComponentTypes, typename Fn, typename... ComponentTypeIds>
void EntityManager::ForEachArchetypeMatching(
    const Fn& fn, ComponentTypeIds... component_type_ids) const {
  const auto all_ids{
      BuildSortedComponentIdArray<ComponentTypes...>(component_type_ids...)};

  for (const auto& archetype : archetypes_) {
    if (!DoesArchetypeMatch(*archetype, all_ids)) {
      continue;
    }

    fn(*archetype);
  }
}
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_ENTITY_MANAGER_ITERATION_TEMPLATE_H_