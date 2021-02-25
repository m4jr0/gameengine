// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "transform.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include <debug_windows.hpp>
#endif  // _WIN32

namespace koma {
const glm::mat4 Transform::GetTransformMatrix() const {
  // TODO(m4jr0): Replace this piece of code with some real transformations.
  return glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
}

void Transform::Destroy() {
  this->parent_ = nullptr;
  this->root_parent_ = nullptr;
}

const glm::vec3 Transform::position() const noexcept {
  return this->position_;
}

void Transform::position(float x, float y, float z) {
  this->position_ = glm::vec3(x, y, z);
}

void Transform::position(glm::vec3 position) {
  this->position_ = position;
}

const std::shared_ptr<Transform> Transform::parent() const noexcept {
  return this->parent_;
}

const std::shared_ptr<Transform> Transform::root_parent() const noexcept {
  return this->root_parent_;
}
}  // namespace koma
