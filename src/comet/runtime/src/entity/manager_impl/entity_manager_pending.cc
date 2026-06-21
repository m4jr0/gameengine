// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/entity/entity_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/fiber/fiber_primitive.h"
#include "comet/core/container/array.h"
#include "comet/runtime/entity/component.h"
#include "comet/runtime/entity/entity_id.h"
#include "comet/runtime/entity/pending_entity.h"

namespace comet {
namespace entity {
usize EntityManager::GetPendingEntityCount() const {
  fiber::FiberLockGuard lock{pending_mutex_};
  return pending_entities_[write_pending_index_].GetEntryCount();
}

internal::PendingEntity& EntityManager::GetOrCreatePendingEntity(
    EntityId entity_id) {
  auto& pending_entities{GetWritePendingEntities()};
  auto* allocator{GetWritePendingAllocator()};

  auto* entity{pending_entities.TryGet(entity_id)};

  if (entity == nullptr) {
    internal::PendingEntity pending{};
    pending.id = entity_id;
    pending.added_cmps = Array<internal::AddedComponent>{allocator};
    pending.removed_cmps = OrderedSet<EntityId>{allocator};

    entity = &pending_entities.Emplace(entity_id, std::move(pending)).value;
  }

  return *entity;
}

bool EntityManager::HasPendingComponentAdd(
    const internal::PendingEntity* pending_entity,
    EntityId component_type_id) const {
  if (pending_entity == nullptr) {
    return false;
  }

  for (const auto& cmp : pending_entity->added_cmps) {
    if (cmp.descr.type_descr.id == component_type_id) {
      return true;
    }
  }

  return false;
}

bool EntityManager::HasPendingComponentRemoval(
    const internal::PendingEntity* pending_entity,
    EntityId component_type_id) const {
  if (pending_entity == nullptr) {
    return false;
  }

  for (const auto removed_id : pending_entity->removed_cmps) {
    if (removed_id == component_type_id) {
      return true;
    }
  }

  return false;
}

const ComponentDescr* EntityManager::FindAddedComponentDescr(
    const Array<internal::AddedComponent>& added_cmps,
    EntityId component_type_id) const {
  for (const auto& cmp : added_cmps) {
    if (cmp.descr.type_descr.id == component_type_id) {
      return &cmp.descr;
    }
  }

  return nullptr;
}

EntityId EntityManager::FindPendingParentId(
    const internal::PendingEntity* pending_entity) const {
  if (pending_entity == nullptr) {
    return kInvalidEntityId;
  }

  EntityId parent_id{kInvalidEntityId};

  for (const auto& cmp : pending_entity->added_cmps) {
    if (!IsParentTag(cmp.descr.type_descr.id)) {
      continue;
    }

    const auto candidate_parent_id{Untag(cmp.descr.type_descr.id)};

    COMET_ASSERT(parent_id == kInvalidEntityId,
                 "EntityManager::FindPendingParentId",
                 "pending entity has more than one parent", "entity_id",
                 pending_entity->id, "first_parent_id", parent_id,
                 "second_parent_id", candidate_parent_id);

    parent_id = candidate_parent_id;
  }

  return parent_id;
}

void EntityManager::DeferRemovingComponents(internal::PendingEntity* entity,
                                            const Array<EntityId>& src) {
  for (const auto& id : src) {
    DeferRemovingComponent(entity, id);
  }
}

void EntityManager::DeferRemovingComponent(internal::PendingEntity* entity,
                                           const EntityId& id) {
  for (usize i{0}; i < entity->added_cmps.GetSize();) {
    if (entity->added_cmps[i].descr.type_descr.id == id) {
      entity->added_cmps.RemoveFromIndex(i);
      return;  // Cancel pending add.
    }

    ++i;
  }

  entity->removed_cmps.Add(id);
}

void EntityManager::DeferAddingParent(internal::PendingEntity* entity,
                                      EntityId parent_id) {
  ComponentTypeDescr type_descr{};
  type_descr.id = Tag(EntityIdTag::Child, parent_id);
  type_descr.size = 0;
  type_descr.align = 0;

  entity->removed_cmps.Remove(type_descr.id);

  for (const auto& added : entity->added_cmps) {
    if (added.descr.type_descr.id == type_descr.id) {
      return;
    }
  }

  entity->added_cmps.EmplaceLast(internal::ComponentKind::Parent,
                                 ComponentDescr{type_descr, nullptr});
}
}  // namespace entity
}  // namespace comet