// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_GAME_OBJECT_MANAGER_HPP_
#define KOMA_CORE_GAME_OBJECT_GAME_OBJECT_MANAGER_HPP_

#include <memory>
#include <unordered_map>

#include "../game_object/game_object.hpp"

namespace koma {
class GameObjectManager {
 public:
  GameObjectManager();
  virtual ~GameObjectManager();

  void Update();
  void FixedUpdate();
  void AddGameObject(std::shared_ptr<GameObject>);
  void RemoveGameObject(std::shared_ptr<GameObject>);

 private:
  std::unordered_map<std::string, std::shared_ptr<GameObject>> game_objects_;
};
};  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_GAME_OBJECT_MANAGER_HPP_
