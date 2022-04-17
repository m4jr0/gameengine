// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GAME_OBJECT_GAME_OBJECT_MANAGER_H_
#define COMET_COMET_GAME_OBJECT_GAME_OBJECT_MANAGER_H_

#include "comet/core/manager.h"
#include "comet/game_object/game_object.h"
#include "comet_precompile.h"

namespace comet {
namespace game_object {
class GameObjectManager : public core::Manager {
 public:
  GameObjectManager() = default;
  GameObjectManager(const GameObjectManager&) = delete;
  GameObjectManager(GameObjectManager&&) = delete;
  GameObjectManager& operator=(const GameObjectManager&) = delete;
  GameObjectManager& operator=(GameObjectManager&&) = delete;
  virtual ~GameObjectManager() = default;

  void Destroy() override;
  void Update() override;
  void FixedUpdate();
  void AddGameObject(std::shared_ptr<GameObject> game_object);
  void RemoveGameObject(std::shared_ptr<GameObject> game_object);

 private:
  std::unordered_map<std::string, std::shared_ptr<GameObject>> game_objects_;
};
}  // namespace game_object
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_GAME_OBJECT_MANAGER_H_
