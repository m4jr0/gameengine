// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allows debugging memory leaks.
#include "../../../debug.hpp"

#include "orthogonal_camera.hpp"

#include <glm/glm.hpp>

namespace koma {
  void OrthogonalCamera::UpdateProjectionMatrix(int width, int height) {
    this->projection_matrix_ = glm::ortho(
      0.0f,
      (float)width,
      (float)height,
      0.0f,
      this->nearest_point_,
      this->farthest_point_
    );
  }
};  // namespace koma
