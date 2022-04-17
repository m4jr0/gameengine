// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "game_object_manager.h"

#include "boost/functional/hash.hpp"
#include "boost/uuid/uuid.hpp"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace game_object {
void GameObjectManager::Destroy() {
  for (const auto& it : game_objects_) {
    it.second->Destroy();
  }
}

void GameObjectManager::Update() {
  for (const auto& it : game_objects_) {
    it.second->Update();
  }
}

void GameObjectManager::FixedUpdate() {
  for (const auto& it : game_objects_) {
    it.second->FixedUpdate();
  }
}

void GameObjectManager::AddGameObject(std::shared_ptr<GameObject> game_object) {
  game_objects_.insert(
      {boost::uuids::to_string(game_object->GetId()), game_object});
}

void GameObjectManager::RemoveGameObject(
    std::shared_ptr<GameObject> game_object) {
  game_objects_.erase(boost::uuids::to_string(game_object->GetId()));
}
}  // namespace game_object
}  // namespace comet
