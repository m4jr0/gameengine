// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "physics_manager.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "comet/core/engine.h"
#include "comet/entity/component/transform_component.h"
#include "comet/entity/entity.h"
#include "comet/entity/entity_manager.h"

namespace comet {
namespace physics {
void PhysicsManager::Initialize(){};
void PhysicsManager::Destroy(){};

void PhysicsManager::Update(entity::EntityManager& entity_manager) {
  current_time_ += Engine::Get().GetTimeManager().GetDeltaTime();

  if (current_time_ > 1000) {
    current_time_ = 0;
    counter_ = 0;
  }

  // TODO(m4jr0): Remove temporary code.
  auto view{entity_manager.GetView<entity::TransformComponent>()};
  static double rot_step = 0;
  rot_step += 0.01;

  for (const auto entity_id : view) {
    auto* transform_cmp{
        entity_manager.GetComponent<entity::TransformComponent>(entity_id)};
    if (transform_cmp->global[0][0] == -1) {
    }

    transform_cmp->global = glm::rotate(
        glm::mat4(1.0f), static_cast<float>(rot_step * glm::radians(90.0f)),
        glm::vec3(0.0f, 1.0f, 0.0f));
  }

  ++counter_;
}

u8 PhysicsManager::GetCounter() const noexcept { return counter_; }
}  // namespace physics
}  // namespace comet
