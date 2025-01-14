// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_PHYSICS_COMPONENT_TRANSFORM_COMPONENT_H_
#define COMET_COMET_PHYSICS_COMPONENT_TRANSFORM_COMPONENT_H_

#include "comet/core/essentials.h"
#include "comet/entity/entity_id.h"
#include "comet/math/matrix.h"
#include "comet/math/quaternion.h"
#include "comet/math/vector.h"

namespace comet {
namespace physics {
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
}  // namespace physics
}  // namespace comet

#endif  // COMET_COMET_PHYSICS_COMPONENT_TRANSFORM_COMPONENT_H_
