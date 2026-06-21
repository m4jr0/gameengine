// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_TRANSFORM_COMPONENT_TRANSFORM_COMPONENT_H_
#define COMET_RUNTIME_TRANSFORM_COMPONENT_TRANSFORM_COMPONENT_H_

#include "comet/core/essentials.h"
#include "comet/runtime/entity/entity_id.h"
#include "comet/core/math/matrix.h"

namespace comet {
namespace transform {
struct TransformRootComponent {
  bool is_child_dirty{false};
};

struct TransformComponent {
  bool is_dirty{false};
  entity::EntityId root_entity_id{entity::kInvalidEntityId};
  entity::EntityId parent_entity_id{entity::kInvalidEntityId};
  math::Mat4 local{1.0f};
  math::Mat4 global{1.0f};
};
}  // namespace transform
}  // namespace comet

#endif  // COMET_RUNTIME_TRANSFORM_COMPONENT_TRANSFORM_COMPONENT_H_
