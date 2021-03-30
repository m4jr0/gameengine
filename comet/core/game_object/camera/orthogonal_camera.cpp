// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "orthogonal_camera.hpp"

#include "glm/glm.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

namespace comet {
void OrthogonalCamera::UpdateProjectionMatrix(int width, int height) {
  projection_matrix_ =
      glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height),
                 0.0f, nearest_point_, farthest_point_);
}
}  // namespace comet
