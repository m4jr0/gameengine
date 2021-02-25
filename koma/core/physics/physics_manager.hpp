// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_PHYSICS_PHYSICS_MANAGER_HPP_
#define KOMA_CORE_PHYSICS_PHYSICS_MANAGER_HPP_

#define LOGGER_KOMA_CORE_PHYSICS_PHYSICS_MANAGER "koma_physics"

#include <core/manager.hpp>
#include <core/game_object/game_object_manager.hpp>

namespace koma {
class PhysicsManager : public Manager {
 public:
  void Initialize() override;
  void Destroy() override;

  void Update(GameObjectManager *);

  const int counter() const noexcept;
 private:
  int counter_ = 0;
  double current_time_ = 0;
};
}  // namespace koma

#endif  // KOMA_CORE_PHYSICS_PHYSICS_MANAGER_HPP_
