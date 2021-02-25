// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "orthogonal_camera.hpp"

#include <glm/glm.hpp>

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include <debug_windows.hpp>
#endif  // _WIN32

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
}  // namespace koma
