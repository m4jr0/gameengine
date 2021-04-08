// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "component.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
void Component::Initialize() {
  if (game_object_ == nullptr) {
    Logger::Get(LoggerType::GameObject)
        ->Error("Cannot initialize a component without a game object");

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

const boost::uuids::uuid Component::kId() const noexcept { return kId_; }

std::shared_ptr<GameObject> Component::game_object() noexcept {
  return game_object_;
}

void Component::game_object(std::shared_ptr<GameObject> game_object) {
  game_object_ = game_object;
}
}  // namespace comet
