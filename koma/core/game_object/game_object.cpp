// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allow debugging memory leaks.
#include "../../debug.hpp"

#include "game_object.hpp"

namespace koma {
GameObject::~GameObject() {
  for (auto it : this->components_) {
    delete it.second;
  }
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

void GameObject::AddComponent(Component *component) {
  this->components_.insert({
    boost::uuids::to_string(component->kId()), component
  });

  component->Initialize();
}

void GameObject::RemoveComponent(Component *component) {
  this->components_.erase(boost::uuids::to_string(component->kId()));
}

const boost::uuids::uuid GameObject::kId() const noexcept {
  return this->kId_;
}
};  // namespace koma
