// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ENTITY_TYPE_ARCHETYPE_H_
#define COMET_RUNTIME_ENTITY_TYPE_ARCHETYPE_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/core/type/map.h"
#include "comet/entity/entity_type.h"
#include "comet/entity/type/entity_id.h"

namespace comet {
namespace entity {
struct ComponentArray {
  u8* elements{nullptr};
  usize size{0};
};

using ArchetypeId = usize;
constexpr auto kInvalidArchetypeId{static_cast<ArchetypeId>(-1)};

struct Archetype {
  ArchetypeId id{kInvalidArchetypeId};
  usize index{kInvalidIndex};
  usize size{0};
  usize capacity{0};
  EntityType entity_type{};
  Array<EntityId> entity_ids{};
  Array<ComponentArray> components{};
};

struct Record {
  Archetype* archetype{nullptr};
  usize row{kInvalidIndex};
};

using Records = Map<EntityId, Record>;

struct ArchetypeRecord {
  usize cmp_array_index{kInvalidIndex};
};

using ArchetypeMap = Map<ArchetypeId, ArchetypeRecord>;
using EntityArchetypeMap = Map<EntityId, Archetype*>;

struct RegisteredComponentType {
  ComponentTypeDescr type_descr{};
  ArchetypeMap archetype_map{};
  usize archetype_ref_count{0};
};

using RegisteredComponentTypeMap = Map<EntityId, RegisteredComponentType>;

using ArchetypePtr = memory::CustomUniquePtr<Archetype>;
}  // namespace entity
}  // namespace comet

#endif  // COMET_RUNTIME_ENTITY_TYPE_ARCHETYPE_H_
