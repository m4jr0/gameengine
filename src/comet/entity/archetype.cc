// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "archetype.h"

#include "comet/core/memory/memory.h"
#include "comet/core/memory/memory_general_alloc.h"

namespace comet {
namespace entity {
ArchetypePointer GenerateArchetype() {
  auto* p{memory::AllocateOneAndPopulate<Archetype>(
      memory::kEngineMemoryTagEntity)};

  ArchetypePointer archetype(p, [](Archetype* p) {
    p->~Archetype();
    memory::Deallocate(p);
  });

  return archetype;
}
}  // namespace entity
}  // namespace comet
