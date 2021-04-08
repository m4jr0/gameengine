// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "game_object.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace game_object {
GameObject::GameObject(GameObject::constructor_tag_){};

GameObject::~GameObject() = default;

std::shared_ptr<GameObject> GameObject::Create() {
  return std::make_shared<GameObject>(GameObject::constructor_tag_{});
}

void GameObject::Destroy() {
  for (const auto &it : components_) {
    it.second->Destroy();
  }
}

void GameObject::Update() {
  for (const auto &it : components_) {
    it.second->Update();
  }
}

void GameObject::FixedUpdate() {
  for (const auto &it : components_) {
    it.second->FixedUpdate();
  }
}

void GameObject::AddComponent(std::shared_ptr<Component> component) {
  components_.insert({boost::uuids::to_string(component->kId()), component});

  component->game_object(shared_from_this());
  component->Initialize();
}

void GameObject::RemoveComponent(std::shared_ptr<Component> component) {
  const auto component_id = boost::uuids::to_string(component->kId());

  components_.at(component_id)->Destroy();
  components_.erase(component_id);
}

std::shared_ptr<Component> GameObject::GetComponent(
    const boost::uuids::uuid id) {
  return components_[boost::uuids::to_string(id)];
}

const boost::uuids::uuid GameObject::kId() const noexcept { return kId_; }
}  // namespace game_object
}  // namespace comet
