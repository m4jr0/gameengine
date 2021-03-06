// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "component.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace game_object {
Component::Component(const Component& other)
    : id_(boost::uuids::random_generator()()), game_object_(nullptr) {}

Component::Component(Component&& other) noexcept : id_(std::move(other.id_)) {}

Component& Component::operator=(const Component& other) {
  if (this == &other) {
    return *this;
  }

  id_ = boost::uuids::random_generator()();
  return *this;
}

Component& Component::operator=(Component&& other) noexcept {
  id_ = std::move(other.id_);
  return *this;
}

void Component::Initialize() {
  if (game_object_ == nullptr) {
    core::Logger::Get(core::LoggerType::GameObject)
        .Error("Cannot initialize a component without a game object");

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

const boost::uuids::uuid& Component::GetId() const noexcept { return id_; }

std::shared_ptr<GameObject> Component::GetGameObject() const noexcept {
  return game_object_;
}

void Component::SetGameObject(std::shared_ptr<GameObject> game_object) {
  game_object_ = game_object;
}
}  // namespace game_object
}  // namespace comet
