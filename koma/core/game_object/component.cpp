// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "component.hpp"

#include <utils/logger.hpp>

// Allow debugging memory leaks.
#include <debug.hpp>

namespace koma {
void Component::Initialize() {
  if (!this->game_object_) {
    Logger::Get(LOGGER_KOMA_CORE_GAME_OBJECT_COMPONENT)->Error(
      "Cannot initialize a component without game objects"
    );

    return;
  }

  // Code has to be implemented in children.
};

void Component::Update() {
  // Code has to be implemented in children.
  // This function is called every frame rendered.
}

void Component::FixedUpdate() {
  // Code has to be implemented in children.
  // This function is called every logic frame computed. In most cases, this
  // is where the game logic will be implemented.
}

const boost::uuids::uuid Component::kId() const noexcept {
  return this->kId_;
}

std::shared_ptr<GameObject> Component::game_object() noexcept {
  return this->game_object_;
}

void Component::game_object(std::shared_ptr<GameObject> game_object) {
  this->game_object_ = game_object;
}
};  // namespace koma
