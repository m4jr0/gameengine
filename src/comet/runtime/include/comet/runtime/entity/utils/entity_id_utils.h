// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ENTITY_UTILS_ENTITY_ID_UTILS_H_
#define COMET_RUNTIME_ENTITY_UTILS_ENTITY_ID_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/core/id/gid.h"
#include "comet/core/id/gid_pool.h"
#include "comet/runtime/entity/entity_id.h"

namespace comet {
namespace entity {
gid::Gid GetGid(EntityId id);

EntityId Tag(EntityIdTag tag, EntityId id);
EntityId Untag(EntityId id);

bool IsTaggedEntityId(EntityId id);
bool IsParentTag(EntityId id);

EntityIdTag GetEntityIdTag(EntityId id);
EntityId GetTaggedEntityIdValue(EntityId id);
}  // namespace entity
}  // namespace comet

#endif  // COMET_RUNTIME_ENTITY_UTILS_ENTITY_ID_UTILS_H_
