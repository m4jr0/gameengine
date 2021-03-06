// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_PHYSICS_PHYSICS_MANAGER_HPP_
#define KOMA_CORE_PHYSICS_PHYSICS_MANAGER_HPP_

constexpr auto kLoggerKomaCorePhysicsPhysicsManager = "koma_physics";

#include "core/game_object/game_object_manager.hpp"
#include "core/manager.hpp"

namespace koma {
class PhysicsManager : public Manager {
 public:
  PhysicsManager() = default;
  PhysicsManager(const PhysicsManager &) = delete;
  PhysicsManager(PhysicsManager &&) = delete;
  PhysicsManager &operator=(const PhysicsManager &) = delete;
  PhysicsManager &operator=(PhysicsManager &&) = delete;
  virtual ~PhysicsManager() = default;

  void Initialize() override;
  void Destroy() override;
  void Update(GameObjectManager *);

  const int counter() const noexcept;

 private:
  unsigned int counter_ = 0;
  double current_time_ = 0;
};
}  // namespace koma

#endif  // KOMA_CORE_PHYSICS_PHYSICS_MANAGER_HPP_
