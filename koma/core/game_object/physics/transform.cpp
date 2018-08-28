// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "transform.hpp"

// Allow debugging memory leaks.
#include <debug.hpp>

namespace koma {
const glm::vec3 Transform::position() const noexcept {
  return this->position_;
}

void Transform::position(float x, float y, float z) {
  this->position_ = glm::vec3(x, y, z);
}

void Transform::position(glm::vec3 position) {
  this->position_ = position;
}
};  // namespace koma
