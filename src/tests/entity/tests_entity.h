// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_TESTS_ENTITY_TESTS_ENTITY_H_
#define COMET_TESTS_ENTITY_TESTS_ENTITY_H_

#include "comet/entity/component/component.h"
#include "comet/entity/entity.h"
#include "comet/entity/entity_manager.h"

namespace comet {
namespace comettests {
struct DummyEmptyComponent {
  static const entity::ComponentTypeId kComponentTypeId;
};

struct DummyMeshComponent {
  static const entity::ComponentTypeId kComponentTypeId;

  void* mesh{nullptr};
  void* material{nullptr};
};

struct DummyTransformComponent {
  static const entity::ComponentTypeId kComponentTypeId;

  f64 position[3]{};
  f64 rotation[3]{};
  f64 scale[3]{};

  entity::EntityId parent{entity::kInvalidEntityId};
  entity::EntityId first{entity::kInvalidEntityId};
  entity::EntityId prev{entity::kInvalidEntityId};
  entity::EntityId next{entity::kInvalidEntityId};
};

struct DummyHpComponent {
  static const entity::ComponentTypeId kComponentTypeId;

  u16 hit_points{0};
  u16 shield_points{0};
  u16 max_hit_points{0};
  u16 max_shield_points{0};
};
}  // namespace comettests
}  // namespace comet

#endif  // COMET_TESTS_ENTITY_TESTS_ENTITY_H_