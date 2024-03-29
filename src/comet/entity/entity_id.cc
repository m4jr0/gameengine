// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entity_id.h"

namespace comet {
namespace entity {
gid::Gid GetGid(EntityId id) { return static_cast<u32>(id); }

EntityId Tag(EntityIdTag tag, EntityId id) {
  return (static_cast<EntityId>(tag) << 32) + static_cast<u32>(id);
}
}  // namespace entity
}  // namespace comet
