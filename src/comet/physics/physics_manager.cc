// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "physics_manager.h"

#include "comet/core/engine.h"

namespace comet {
namespace physics {
void PhysicsManager::Initialize(){};
void PhysicsManager::Destroy(){};

void PhysicsManager::Update(entity::EntityManager& entity_manager) {
  current_time_ += Engine::Get().GetTimeManager().GetTimeDelta();

  if (current_time_ > 1000) {
    current_time_ = 0;
    counter_ = 0;
  }

  ++counter_;
}

u8 PhysicsManager::GetCounter() const noexcept { return counter_; }
}  // namespace physics
}  // namespace comet
