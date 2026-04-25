// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "entity_type.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/algorithm/sort.h"
#include "comet/entity/entity_memory_context.h"

namespace comet {
namespace entity {
EntityType GenerateEntityType(const Array<ComponentDescr>& component_descrs) {
  EntityType entity_type{&EntityMemoryContext::Get().GetEntityTypeAllocator()};
  entity_type.Reserve(component_descrs.GetSize());

  for (const auto& descr : component_descrs) {
    entity_type.PushLast(descr.type_descr.id);
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
  EntityType new_type{&EntityMemoryContext::Get().GetEntityTypeAllocator()};
  new_type.Reserve(entity_type.GetSize() + to_add.GetSize());

  for (usize i{0}; i < entity_type.GetSize(); ++i) {
    new_type.PushLast(entity_type[i]);
  }

  for (usize i{0}; i < new_type.GetCapacity() - entity_type.GetSize(); ++i) {
    new_type.PushLast(to_add[i]);
  }

  CleanEntityType(new_type);
  return new_type;
}

EntityType RemoveFromEntityType(const EntityType& from_entity_type,
                                const EntityType& to_remove) {
  EntityType entity_type{&EntityMemoryContext::Get().GetEntityTypeAllocator()};
  entity_type.Reserve(from_entity_type.GetSize());

  usize remove_index{0};

  for (usize i{0}; i < from_entity_type.GetSize(); ++i) {
    const auto component_id{from_entity_type[i]};

    // Entity types are sorted.
    while (remove_index < to_remove.GetSize() &&
           to_remove[remove_index] < component_id) {
      ++remove_index;
    }

    if (remove_index < to_remove.GetSize() &&
        to_remove[remove_index] == component_id) {
      continue;
    }

    entity_type.PushLast(component_id);
  }

  return entity_type;
}

EntityType& CleanEntityType(EntityType& entity_type) {
  Sort(entity_type.begin(), entity_type.end());
  return entity_type;
}
}  // namespace entity
}  // namespace comet
