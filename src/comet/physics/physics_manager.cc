// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "physics_manager.h"

#include "comet/core/engine.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace physics {
void PhysicsManager::Initialize(){};
void PhysicsManager::Destroy(){};

void PhysicsManager::Update(
    game_object::GameObjectManager *game_object_manager) {
  current_time_ += core::Engine::engine()->time_manager()->time_delta();

  if (current_time_ > 1000) {
    current_time_ = 0;
    counter_ = 0;
  }

  game_object_manager->FixedUpdate();
  ++counter_;
}

const int PhysicsManager::counter() const noexcept { return counter_; }
}  // namespace physics
}  // namespace comet
