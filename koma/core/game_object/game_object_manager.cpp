// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allows debugging memory leaks.
// TODO(m4jr0): Find a better solution.
#ifdef _DEBUG
#include "../../debug.hpp"
#endif  // _DEBUG

#include "game_object_manager.hpp"

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <functional>

namespace koma {
GameObjectManager::GameObjectManager() {}

GameObjectManager::~GameObjectManager() {
  for (auto it : this->game_objects_) {
    delete it.second;
  }
}

void GameObjectManager::Update(double interpolation) {
  for (auto it : this->game_objects_) {
    it.second->Update(interpolation);
  }
}

void GameObjectManager::FixedUpdate() {
  for (auto it : this->game_objects_) {
    it.second->FixedUpdate();
  }
}

void GameObjectManager::AddGameObject(GameObject *game_object) {
  this->game_objects_.insert({
    boost::uuids::to_string(game_object->kId()), game_object
  });
}

void GameObjectManager::RemoveGameObject(GameObject *game_object) {
  this->game_objects_.erase(boost::uuids::to_string(game_object->kId()));
}
};  // namespace koma
