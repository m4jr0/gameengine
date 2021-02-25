// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "component.hpp"

#include <utils/logger.hpp>

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include <debug_windows.hpp>
#endif  // _WIN32

namespace koma {
void Component::Initialize() {
  if (!this->game_object_) {
    Logger::Get(LOGGER_KOMA_CORE_GAME_OBJECT_COMPONENT)->Error(
      "Cannot initialize a component without a game object"
    );

    return;
  }

  // Code has to be implemented in children.
};

void Component::Destroy() {
  // Code has to be implemented in children.
  // This function is called every frame rendered.
}

void Component::Update() {
  // Code has to be implemented in children.
  // This function is called every frame rendered.
}

void Component::FixedUpdate() {
  // Code has to be implemented in children.
  // This function is called every logic frame computed.
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
}  // namespace koma
