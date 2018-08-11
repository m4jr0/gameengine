// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_MANAGER_HPP_
#define KOMA_CORE_GAME_OBJECT_MANAGER_HPP_

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <functional>
#include <unordered_map>

#include "../game_object/game_object.hpp"

namespace koma {
class GameObjectManager {
 public:
  GameObjectManager();
  virtual ~GameObjectManager();

  void Update(double);
  void FixedUpdate();
  void AddGameObject(GameObject*);
  void RemoveGameObject(GameObject*);

 private:
  std::unordered_map<std::string, GameObject *> game_objects_;
};
};  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_MANAGER_HPP_
