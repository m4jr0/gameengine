// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ARCHETYPE_H_
#define COMET_COMET_ENTITY_ARCHETYPE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/stl_types.h"
#include "comet/entity/entity_id.h"
#include "comet/entity/entity_type.h"

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
  EntityType entity_type{};
  std::vector<EntityId> entity_ids{};
  std::vector<ComponentArray> components{};
};

struct Record {
  Archetype* archetype{nullptr};
  usize row{kInvalidIndex};
};

using Records = std::unordered_map<EntityId, Record>;

struct ArchetypeRecord {
  usize cmp_array_index{kInvalidIndex};
};

using ArchetypeMap = std::unordered_map<ArchetypeId, ArchetypeRecord>;
using EntityArchetypeMap = std::unordered_map<EntityId, Archetype*>;

struct RegisteredComponentType {
  ComponentTypeDescr type_descr{};
  ArchetypeMap archetype_map{};
  usize use_count{0};
};

using RegisteredComponentTypeMap =
    std::unordered_map<EntityId, RegisteredComponentType>;

using ArchetypePointer = custom_unique_ptr<Archetype>;

ArchetypePointer GenerateArchetype();
}  // namespace entity
}  // namespace comet
#endif  // COMET_COMET_ENTITY_ARCHETYPE_H_
