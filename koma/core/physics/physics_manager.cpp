// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "physics_manager.hpp"

namespace koma {
void PhysicsManager::Initialize() {};
void PhysicsManager::Destroy() {};

void PhysicsManager::Update(GameObjectManager *game_object_manager) {
  this->current_time_ += Locator::time_manager().time_delta();

  if (this->current_time_ > 1000) {
    Logger::Get(LOGGER_KOMA_CORE_PHYSICS)->Debug(
      "Physics ", this->counter_
    );

    this->current_time_ = 0;
    this->counter_ = 0;
  }

  game_object_manager->FixedUpdate();
  ++this->counter_;
}
};  // namespace koma
