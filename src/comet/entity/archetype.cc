// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "archetype.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/entity/entity_memory_manager.h"

namespace comet {
namespace entity {
ArchetypePtr GenerateArchetype() {
  auto& memory_manager{EntityMemoryManager::Get()};

  auto* p{memory_manager.GetArchetypeAllocator()
              .AllocateOneAndPopulate<Archetype>()};

  p->entity_type = EntityType{&memory_manager.GetEntityTypeAllocator()};
  p->entity_ids = Array<EntityId>{&memory_manager.GetEntityIdAllocator()};
  p->components =
      Array<ComponentArray>{&memory_manager.GetComponentArrayAllocator()};

  ArchetypePtr archetype{
      p, [](Archetype* ptr) {
        ptr->~Archetype();
        EntityMemoryManager::Get().GetArchetypeAllocator().Deallocate(ptr);
      }};

  return archetype;
}
}  // namespace entity
}  // namespace comet
