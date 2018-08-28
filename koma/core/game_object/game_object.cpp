// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "game_object.hpp"

// Allow debugging memory leaks.
#include <debug.hpp>

namespace koma {
GameObject::~GameObject() = default;

std::shared_ptr<GameObject> GameObject::Create() {
  return std::make_shared<GameObject>();
}

void GameObject::Update() {
  for (auto it : this->components_) {
    it.second->Update();
  }
}

void GameObject::FixedUpdate() {
  for (auto it : this->components_) {
    it.second->FixedUpdate();
  }
}

void GameObject::AddComponent(std::shared_ptr<Component> component) {
  this->components_.insert({
    boost::uuids::to_string(component->kId()), component
  });

  component->game_object(this->shared_from_this());
  component->Initialize();
}

void GameObject::RemoveComponent(std::shared_ptr<Component> component) {
  this->components_.erase(boost::uuids::to_string(component->kId()));
}

std::shared_ptr<Component> GameObject::GetComponent(
  const boost::uuids::uuid id) {
  return this->components_[boost::uuids::to_string(id)];
}

const boost::uuids::uuid GameObject::kId() const noexcept {
  return this->kId_;
}

GameObject::GameObject() = default;
};  // namespace koma
