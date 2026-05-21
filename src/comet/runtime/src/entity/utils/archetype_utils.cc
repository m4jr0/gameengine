// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/entity/utils/archetype_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type/array.h"
#include "comet/entity/entity_memory_context.h"
#include "comet/entity/entity_type.h"
#include "comet/entity/type/entity_id.h"

namespace comet {
namespace entity {
ArchetypePtr GenerateArchetype() {
  auto& memory_context{EntityMemoryContext::Get()};

  auto* p{memory_context.GetArchetypeAllocator()
              .AllocateOneAndPopulate<Archetype>()};

  p->entity_type = EntityType{&memory_context.GetEntityTypeAllocator()};
  p->entity_ids = Array<EntityId>{&memory_context.GetEntityIdAllocator()};
  p->components =
      Array<ComponentArray>{&memory_context.GetComponentArrayAllocator()};

  ArchetypePtr archetype{
      p, [](Archetype* ptr) {
        ptr->~Archetype();
        EntityMemoryContext::Get().GetArchetypeAllocator().Deallocate(ptr);
      }};

  return archetype;
}
}  // namespace entity
}  // namespace comet
