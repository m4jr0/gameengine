// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "perspective_camera.hpp"

#include <glm/glm.hpp>

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include <debug_windows.hpp>
#endif  // _WIN32

namespace koma {
void PerspectiveCamera::UpdateProjectionMatrix(int width, int height) {
  this->projection_matrix_ = glm::perspective(
    glm::radians(this->fov_),
    (float)width / (float)height,
    this->nearest_point_,
    this->farthest_point_
  );
}

void PerspectiveCamera::fov(float fov) noexcept {
  this->fov_ = fov;
}

const float PerspectiveCamera::fov() const noexcept {
  return this->fov_;
}
}  // namespace koma
