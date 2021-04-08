// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "perspective_camera.h"

#include "glm/glm.hpp"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace game_object {
void PerspectiveCamera::UpdateProjectionMatrix(int width, int height) {
  projection_matrix_ =
      glm::perspective(glm::radians(fov_),
                       static_cast<float>(width) / static_cast<float>(height),
                       nearest_point_, farthest_point_);
}

void PerspectiveCamera::fov(float fov) noexcept { fov_ = fov; }

const float PerspectiveCamera::fov() const noexcept { return fov_; }
}  // namespace game_object
}  // namespace comet
