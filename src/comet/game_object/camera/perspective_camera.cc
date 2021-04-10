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
PerspectiveCamera::PerspectiveCamera(const PerspectiveCamera& other)
    : Camera(other), fov_(other.fov_) {}

PerspectiveCamera::PerspectiveCamera(PerspectiveCamera&& other) noexcept
    : Camera(std::move(other)), fov_(std::move(other.fov_)) {}

PerspectiveCamera& PerspectiveCamera::operator=(
    const PerspectiveCamera& other) {
  if (this == &other) {
    return *this;
  }

  Camera::operator=(other);
  fov_ = other.fov_;
  return *this;
}

PerspectiveCamera& PerspectiveCamera::operator=(
    PerspectiveCamera&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Camera::operator=(std::move(other));
  fov_ = std::move(other.fov_);
  return *this;
}

std::shared_ptr<Component> PerspectiveCamera::Clone() const {
  return std::make_shared<PerspectiveCamera>(*this);
}

void PerspectiveCamera::UpdateProjectionMatrix(int width, int height) {
  projection_matrix_ =
      glm::perspective(glm::radians(fov_),
                       static_cast<float>(width) / static_cast<float>(height),
                       nearest_point_, farthest_point_);
}

float PerspectiveCamera::GetFov() const noexcept { return fov_; }

void PerspectiveCamera::SetFov(float fov) noexcept { fov_ = fov; }
}  // namespace game_object
}  // namespace comet
