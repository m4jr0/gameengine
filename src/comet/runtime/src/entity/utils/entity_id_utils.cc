// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/entity/utils/entity_id_utils.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace entity {
gid::Gid GetGid(EntityId id) { return static_cast<u32>(id); }

EntityId Tag(EntityIdTag tag, EntityId id) {
  return (static_cast<EntityId>(tag) << 32) | static_cast<u32>(id);
}

EntityId Untag(EntityId id) {
  return static_cast<EntityId>(static_cast<u32>(id));
}

bool IsTaggedEntityId(EntityId id) { return (static_cast<u64>(id) >> 32) != 0; }

bool IsParentTag(EntityId id) {
  return IsTaggedEntityId(id) && GetEntityIdTag(id) == EntityIdTag::Child;
}

EntityIdTag GetEntityIdTag(EntityId id) {
  return static_cast<EntityIdTag>(static_cast<u64>(id) >> 32);
}

EntityId GetTaggedEntityIdValue(EntityId id) {
  return static_cast<EntityId>(static_cast<u32>(id));
}
}  // namespace entity
}  // namespace comet
