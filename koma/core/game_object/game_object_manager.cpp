// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "game_object_manager.hpp"

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <functional>

// Allow debugging memory leaks.
#include <debug.hpp>

namespace koma {
GameObjectManager::GameObjectManager() {}

GameObjectManager::~GameObjectManager() = default;

void GameObjectManager::Update() {
  for (auto it : this->game_objects_) {
    it.second->Update();
  }
}

void GameObjectManager::FixedUpdate() {
  for (auto it : this->game_objects_) {
    it.second->FixedUpdate();
  }
}

void GameObjectManager::AddGameObject(
  std::shared_ptr<GameObject> game_object) {
  this->game_objects_.insert({
    boost::uuids::to_string(game_object->kId()), game_object
  });
}

void GameObjectManager::RemoveGameObject(
  std::shared_ptr<GameObject> game_object) {
  this->game_objects_.erase(boost::uuids::to_string(game_object->kId()));
}
};  // namespace koma
