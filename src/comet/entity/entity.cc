// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entity.h"

namespace comet {
namespace entity {
constexpr EntityId Entity::GetIndex() const noexcept {
  return gid::GetIndex(id);
}

constexpr EntityId Entity::GetGeneration() const noexcept {
  return gid::GetGeneration(id);
}
}  // namespace entity
}  // namespace comet
