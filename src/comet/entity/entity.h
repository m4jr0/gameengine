// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_H_
#define COMET_COMET_ENTITY_ENTITY_H_

#include "comet_precompile.h"

namespace comet {
namespace entity {
using EntityId = gid::Gid;
constexpr auto kInvalidEntityId{gid::kInvalidId};
constexpr auto kMaxEntityComponentCount{10};

struct Entity {
  EntityId id{kInvalidEntityId};
  constexpr EntityId GetIndex() const noexcept;
  constexpr EntityId GetGeneration() const noexcept;
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_ENTITY_H_
