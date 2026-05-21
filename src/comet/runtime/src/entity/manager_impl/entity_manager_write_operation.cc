// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/entity/entity_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/type/array.h"
#include "comet/entity/type/entity_id.h"
#include "comet/entity/type/pending_entity.h"

namespace comet {
namespace entity {
EntityId EntityManager::Generate() {
  EntityId new_entity_id{kInvalidEntityId};
  {
    fiber::FiberSharedLockGuard lock{entity_id_mutex_,
                                     fiber::FiberSharedLockType::Exclusive};
    new_entity_id = entity_id_handler_.Generate();
  }

  fiber::FiberLockGuard lock{pending_mutex_};

  auto& pending_entities{GetWritePendingEntities()};
  auto* allocator{GetWritePendingAllocator()};

  auto pending{internal::PendingEntity{false, new_entity_id}};
  pending.added_cmps = Array<internal::AddedComponent>{allocator};
  pending.removed_cmps = OrderedSet<EntityId>{allocator};

  pending_entities.Emplace(new_entity_id, std::move(pending));
  return new_entity_id;
}

void EntityManager::Destroy(EntityId entity_id) {
  COMET_ASSERT(IsEntityUnlocked(entity_id), "EntityManager::Destroy",
               "entity does not exist", "entity_id", entity_id);

  {
    fiber::FiberLockGuard lock{pending_mutex_};
    auto& entity{GetOrCreatePendingEntity(entity_id)};
    entity.is_destroyed = true;
    entity.added_cmps.Release();
    entity.removed_cmps.Release();
  }

  ForEachChildId<>([&](auto child_entity_id) { Destroy(child_entity_id); },
                   entity_id);
}

void EntityManager::RemoveComponents(EntityId entity_id,
                                     const Array<EntityId>& component_ids) {
  COMET_ASSERT(entity_id != kInvalidEntityId, "EntityManager::RemoveComponents",
               "invalid entity id");

  fiber::FiberLockGuard lock{pending_mutex_};
  auto& entity{GetOrCreatePendingEntity(entity_id)};

  COMET_ASSERT(!entity.is_destroyed, "EntityManager::RemoveComponents",
               "entity scheduled for destruction", "entity_id", entity_id);

  DeferRemovingComponents(&entity, component_ids);
}

void EntityManager::AddParent(EntityId entity_id, EntityId parent_id) {
  COMET_ASSERT(entity_id != kInvalidEntityId, "EntityManager::AddParent",
               "invalid entity id");
  COMET_ASSERT(parent_id != kInvalidEntityId, "EntityManager::AddParent",
               "invalid parent id");
  COMET_ASSERT(entity_id != parent_id, "EntityManager::AddParent",
               "entity cannot parent itself", "entity_id", entity_id,
               "parent_id", parent_id);

  fiber::FiberLockGuard lock{pending_mutex_};

  auto& entity{GetOrCreatePendingEntity(entity_id)};
  COMET_ASSERT(!entity.is_destroyed, "EntityManager::AddParent",
               "entity scheduled for destruction", "entity_id", entity_id);

  DeferAddingParent(&entity, parent_id);
}
}  // namespace entity
}  // namespace comet