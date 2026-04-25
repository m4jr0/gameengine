// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/entity/entity_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/type/array.h"
#include "comet/entity/entity_type.h"
#include "comet/entity/type/archetype.h"
#include "comet/entity/type/entity_id.h"
#include "comet/entity/type/pending_entity.h"

namespace comet {
namespace entity {
bool EntityManager::IsEntity(const EntityId& entity_id) const {
  return ReadSnapshot("EntityManager::IsEntity",
                      [&] { return IsEntityUnlocked(entity_id); });
}

bool EntityManager::HasComponent(EntityId entity_id,
                                 EntityId component_id) const {
  return ReadSnapshot("EntityManager::HasComponent", [&] {
    return HasComponentUnlocked(entity_id, component_id);
  });
}

EntityId EntityManager::GetParentId(EntityId entity_id) const {
  return ReadSnapshot("EntityManager::GetParentId",
                      [&] { return GetParentIdUnlocked(entity_id); });
}

bool EntityManager::HasParent(EntityId entity_id, EntityId parent_id) const {
  return ReadSnapshot("EntityManager::HasParent",
                      [&] { return HasParentUnlocked(entity_id, parent_id); });
}

bool EntityManager::HasAnyParent(EntityId entity_id) const {
  return ReadSnapshot("EntityManager::HasAnyParent",
                      [&] { return HasAnyParentUnlocked(entity_id); });
}

EntityId EntityManager::FindParentId(const EntityType* entity_type) const {
  return ReadSnapshot("EntityManager::FindParentId",
                      [&] { return FindParentIdUnlocked(entity_type); });
}

bool EntityManager::IsDescendant(EntityId entity_id,
                                 EntityId potential_ancestor_id) const {
  return ReadSnapshot("EntityManager::IsDescendant", [&] {
    return IsDescendantUnlocked(entity_id, potential_ancestor_id);
  });
}

usize EntityManager::GetEntityCount() const {
  return ReadSnapshot("EntityManager::GetEntityCount",
                      [&] { return GetEntityCountUnlocked(); });
}

usize EntityManager::GetEntityCapacity() const {
  return ReadSnapshot("EntityManager::GetEntityCapacity",
                      [&] { return GetEntityCapacityUnlocked(); });
}

const EntityType* EntityManager::TryGetEntityType(EntityId entity_id) const {
  return ReadSnapshot("EntityManager::TryGetEntityType",
                      [&] { return TryGetEntityTypeUnlocked(entity_id); });
}

bool EntityManager::IsEntityUnlocked(const EntityId& entity_id) const {
  if (entity_id == kInvalidEntityId) {
    return false;
  }

  fiber::FiberSharedLockGuard lock{entity_id_mutex_,
                                   fiber::FiberSharedLockType::Shared};
  return entity_id_handler_.IsAlive(GetGid(entity_id));
}

bool EntityManager::HasComponentUnlocked(EntityId entity_id,
                                         EntityId component_id) const {
  if (!IsEntityUnlocked(entity_id)) {
    return false;
  }

  const auto* entity_type{TryGetEntityTypeUnlocked(entity_id)};
  return entity_type != nullptr && entity_type->IsContained(component_id);
}

EntityId EntityManager::GetParentIdUnlocked(EntityId entity_id) const {
  COMET_ASSERT(IsEntityUnlocked(entity_id),
               "EntityManager::GetParentIdUnlocked", "entity does not exist",
               "entity_id", entity_id);

  {
    fiber::FiberLockGuard lock{pending_mutex_};

    const auto* pending_entity{
        pending_entities_[write_pending_index_].TryGet(entity_id)};

    const auto pending_parent_id{FindPendingParentId(pending_entity)};

    if (pending_parent_id != kInvalidEntityId) {
      return pending_parent_id;
    }

    if (pending_entity != nullptr) {
      for (const auto removed_id : pending_entity->removed_cmps) {
        if (IsParentTag(removed_id)) {
          return kInvalidEntityId;
        }
      }
    }
  }

  return FindParentIdUnlocked(TryGetEntityTypeUnlocked(entity_id));
}

bool EntityManager::HasParentUnlocked(EntityId entity_id,
                                      EntityId parent_id) const {
  return GetParentIdUnlocked(entity_id) == parent_id;
}

bool EntityManager::HasAnyParentUnlocked(EntityId entity_id) const {
  return GetParentIdUnlocked(entity_id) != kInvalidEntityId;
}

EntityId EntityManager::FindParentIdUnlocked(
    const EntityType* entity_type) const {
  if (entity_type == nullptr) {
    return kInvalidEntityId;
  }

  EntityId parent_id{kInvalidEntityId};

  for (const auto component_type_id : *entity_type) {
    if (!IsParentTag(component_type_id)) {
      continue;
    }

    const auto candidate_parent_id{Untag(component_type_id)};

    COMET_ASSERT(parent_id == kInvalidEntityId,
                 "EntityManager::FindParentIdUnlocked",
                 "entity type has more than one parent", "first_parent_id",
                 parent_id, "second_parent_id", candidate_parent_id);

    parent_id = candidate_parent_id;
  }

  return parent_id;
}

bool EntityManager::IsDescendantUnlocked(EntityId entity_id,
                                         EntityId potential_ancestor_id) const {
  frame::FrameArray<EntityId> stack{};
  constexpr usize kInitialHierarchyStackCapacity{8};
  stack.Reserve(kInitialHierarchyStackCapacity);
  stack.PushLast(potential_ancestor_id);

  while (!stack.IsEmpty()) {
    const auto current{stack.TakeLast()};
    auto found{false};

    ForEachChildIdUnlocked<>(
        [&](EntityId child_id) {
          if (child_id == entity_id) {
            found = true;
            return;
          }

          stack.PushLast(child_id);
        },
        current);

    if (found) {
      return true;
    }
  }

  return false;
}

const EntityType* EntityManager::TryGetEntityTypeUnlocked(
    EntityId entity_id) const {
  const auto* record{records_.TryGet(entity_id)};

  if (record == nullptr || record->archetype == nullptr) {
    return nullptr;
  }

  return &record->archetype->entity_type;
}

usize EntityManager::GetEntityCountUnlocked() const {
  usize count{0};

  for (const auto& archetype : archetypes_) {
    count += archetype->size;
  }

  return count;
}

usize EntityManager::GetEntityCapacityUnlocked() const {
  usize count{0};

  for (const auto& archetype : archetypes_) {
    count += archetype->capacity;
  }

  return count;
}
}  // namespace entity
}  // namespace comet