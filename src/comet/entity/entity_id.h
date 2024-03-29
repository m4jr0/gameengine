// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_ID_H_
#define COMET_COMET_ENTITY_ENTITY_ID_H_

#include "comet_precompile.h"

namespace comet {
namespace entity {
using EntityId = u64;
constexpr auto kInvalidEntityId{static_cast<EntityId>(-1)};
constexpr auto kMaxEntityComponentCount{10};

constexpr auto kMaxComponentId{static_cast<u8>(-1) + 1};

enum EntityIdTag : u32 {
  None = 0,
  Component = static_cast<u32>(1) << 31,
  Child = static_cast<u32>(1) << 30
};

gid::Gid GetGid(EntityId id);
EntityId Tag(EntityIdTag tag, EntityId id);
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_ENTITY_ID_H_
