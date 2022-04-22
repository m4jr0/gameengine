// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_PHYSICS_PHYSICS_MANAGER_H_
#define COMET_COMET_PHYSICS_PHYSICS_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/manager.h"
#include "comet/game_object/game_object_manager.h"

namespace comet {
namespace physics {
class PhysicsManager : public core::Manager {
 public:
  PhysicsManager() = default;
  PhysicsManager(const PhysicsManager&) = delete;
  PhysicsManager(PhysicsManager&&) = delete;
  PhysicsManager& operator=(const PhysicsManager&) = delete;
  PhysicsManager& operator=(PhysicsManager&&) = delete;
  virtual ~PhysicsManager() = default;

  void Initialize() override;
  void Destroy() override;
  void Update(game_object::GameObjectManager& game_object_manager);

  unsigned int GetCounter() const noexcept;

 private:
  unsigned int counter_ = 0;
  double current_time_ = 0;
};
}  // namespace physics
}  // namespace comet

#endif  // COMET_COMET_PHYSICS_PHYSICS_MANAGER_H_
