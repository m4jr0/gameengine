// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_PHYSICS_PHYSICS_MANAGER_H_
#define COMET_COMET_PHYSICS_PHYSICS_MANAGER_H_

#include "comet_precompile.h"

#include "comet/entity/entity_manager.h"

namespace comet {
namespace physics {
class PhysicsManager {
 public:
  PhysicsManager() = default;
  PhysicsManager(const PhysicsManager&) = delete;
  PhysicsManager(PhysicsManager&&) = delete;
  PhysicsManager& operator=(const PhysicsManager&) = delete;
  PhysicsManager& operator=(PhysicsManager&&) = delete;
  ~PhysicsManager() = default;

  void Initialize();
  void Destroy();
  void Update(entity::EntityManager& entity_manager);

  u8 GetCounter() const noexcept;

 private:
  u8 counter_{0};
  f64 current_time_{0};
};
}  // namespace physics
}  // namespace comet

#endif  // COMET_COMET_PHYSICS_PHYSICS_MANAGER_H_
