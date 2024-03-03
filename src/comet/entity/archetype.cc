// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "archetype.h"

#include "comet/core/memory/memory.h"

namespace comet {
namespace entity {
ArchetypePointer GenerateArchetype() {
  auto* p{AllocateAligned<Archetype>(MemoryTag::Entity)};

  ArchetypePointer archetype(p, [](Archetype* p) {
    p->~Archetype();
    Deallocate(p);
  });

  new (archetype.get()) Archetype();
  return archetype;
}
}  // namespace entity
}  // namespace comet
