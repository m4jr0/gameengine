// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_COMPONENT_TRANSFORM_COMPONENT_H_
#define COMET_COMET_ENTITY_COMPONENT_TRANSFORM_COMPONENT_H_

#include "comet_precompile.h"

#include "glm/glm.hpp"

#include "comet/entity/component/component.h"
#include "comet/entity/entity.h"

namespace comet {
namespace entity {
struct TransformComponent {
  static const ComponentTypeId kComponentTypeId;

  EntityId first{kInvalidEntityId};
  EntityId prev{kInvalidEntityId};
  EntityId next{kInvalidEntityId};
  EntityId parent{kInvalidEntityId};
  glm::mat4x4 local{};
  glm::mat4x4 global{};
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_COMPONENT_TRANSFORM_COMPONENT_H_
