// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entity_misc_types.h"

namespace comet {
namespace entity {
EntityType& CleanEntityType(EntityType& entity_type) {
  std::sort(entity_type.begin(), entity_type.end());
  return entity_type;
}
}  // namespace entity
}  // namespace comet
