// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_PHYSICS_MANAGER_HPP_
#define KOMA_CORE_PHYSICS_MANAGER_HPP_

#include "game_object_manager.hpp"

namespace koma {
class PhysicsManager {
 public:
  void Update(GameObjectManager *);
};
}; //  namespace koma

#endif //  KOMA_CORE_PHYSICS_MANAGER_HPP_
