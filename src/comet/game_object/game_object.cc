// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "game_object.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace game_object {
std::shared_ptr<GameObject> GameObject::Clone() const {
  auto clone = GameObject::Create();
  std::shared_ptr<Component> component = nullptr;

  for (const auto& it : components_) {
    component = it.second->Clone();
    clone->AddComponent(component);
  }

  return clone;
}

std::shared_ptr<GameObject> GameObject::Create() {
  return std::make_shared<GameObject>();
}

void GameObject::Destroy() {
  for (const auto& it : components_) {
    it.second->Destroy();
  }
}

void GameObject::Update() {
  for (const auto& it : components_) {
    it.second->Update();
  }
}

void GameObject::FixedUpdate() {
  for (const auto& it : components_) {
    it.second->FixedUpdate();
  }
}

void GameObject::AddComponent(std::shared_ptr<Component> component) {
  components_.insert({boost::uuids::to_string(component->GetId()), component});

  component->SetGameObject(shared_from_this());
  component->Initialize();
}

void GameObject::RemoveComponent(std::shared_ptr<Component> component) {
  const auto component_id = boost::uuids::to_string(component->GetId());

  components_.at(component_id)->Destroy();
  components_.erase(component_id);
}

std::shared_ptr<Component> GameObject::GetComponent(
    const boost::uuids::uuid& id) {
  return components_[boost::uuids::to_string(id)];
}

const boost::uuids::uuid GameObject::GetId() const noexcept { return id_; }
}  // namespace game_object
}  // namespace comet
