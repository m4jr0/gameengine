// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "physics_manager.hpp"

#include "core/game.hpp"
#include "utils/logger.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

namespace koma {
void PhysicsManager::Initialize(){};
void PhysicsManager::Destroy(){};

void PhysicsManager::Update(GameObjectManager *game_object_manager) {
  current_time_ += Game::game()->time_manager()->time_delta();

  if (current_time_ > 1000) {
    // TODO(m4jr0): Uncomment this when debugging the physics module.
    // Logger::Get(LOGGER_KOMA_CORE_PHYSICS_PHYSICS_MANAGER)->Debug(
    //   "Physics ", counter_
    // );

    current_time_ = 0;
    counter_ = 0;
  }

  game_object_manager->FixedUpdate();
  ++counter_;
}

const int PhysicsManager::counter() const noexcept { return counter_; }
}  // namespace koma
