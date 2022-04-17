// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "transform.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace game_object {
Transform::Transform(const Transform& other)
    : Component(other),
      position_(other.position_),
      parent_(other.parent_),
      root_parent_(other.root_parent_) {
  std::shared_ptr<Transform> transform = nullptr;
  children_ = std::unordered_map<std::string, std::shared_ptr<Transform>>();

  for (const auto& it : other.children_) {
    transform = std::make_shared<Transform>(*it.second);
    children_.insert({boost::uuids::to_string(transform->GetId()), transform});
  }
}

Transform::Transform(Transform&& other) noexcept
    : Component(std::move(other)),
      position_(other.position_),
      parent_(other.parent_),
      root_parent_(other.root_parent_),
      children_(std::move(other.children_)) {}

Transform& Transform::operator=(const Transform& other) {
  if (this == &other) {
    return *this;
  }

  Component::operator=(other);
  position_ = other.position_;
  parent_ = other.parent_;
  root_parent_ = other.root_parent_;
  std::shared_ptr<Transform> transform = nullptr;

  children_ = std::unordered_map<std::string, std::shared_ptr<Transform>>();

  for (const auto& it : other.children_) {
    transform = std::make_shared<Transform>(*it.second);
    children_.insert({boost::uuids::to_string(transform->GetId()), transform});
  }

  return *this;
}

Transform& Transform::operator=(Transform&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Component::operator=(std::move(other));
  position_ = other.position_;
  parent_ = other.parent_;
  root_parent_ = other.root_parent_;
  children_ = std::move(other.children_);
  return *this;
}

std::shared_ptr<Component> Transform::Clone() const {
  return std::make_shared<Transform>(*this);
}

void Transform::Destroy() {
  parent_ = nullptr;
  root_parent_ = nullptr;
}

const glm::mat4 Transform::GetTransformMatrix() const {
  // TODO(m4jr0): Replace this piece of code with some real transformations.
  return glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
}

const glm::vec3& Transform::GetPosition() const noexcept { return position_; }

void Transform::SetPosition(float x, float y, float z) {
  position_ = glm::vec3(x, y, z);
}

void Transform::SetPosition(glm::vec3 position) { position_ = position; }

std::shared_ptr<Transform> Transform::GetParent() noexcept { return parent_; }

std::shared_ptr<Transform> Transform::GetRootParent() noexcept {
  return root_parent_;
}
}  // namespace game_object
}  // namespace comet
