// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allows debugging memory leaks.
#include "../../../debug.hpp"

#include "perspective_camera.hpp"

namespace koma {
void PerspectiveCamera::UpdateProjectionMatrix(GLuint width, GLuint height) {
  this->projection_matrix_ = glm::perspective(
    glm::radians(this->fov_),
    (float)width / (float)height,
    this->nearest_point_,
    this->farthest_point_
  );
}

void PerspectiveCamera::fov(float fov) {
  this->fov_ = fov;
}

const float PerspectiveCamera::fov() const noexcept {
  return this->fov_;
}
};  // namespace koma
