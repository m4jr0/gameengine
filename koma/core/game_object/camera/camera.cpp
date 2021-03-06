// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera.hpp"

#include "core/game.hpp"
#include "core/render/render_manager.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

namespace koma {
void Camera::Initialize() {
  Component::Initialize();

  const auto render_manager = Game::game()->render_manager();

  UpdateMatrices(render_manager->width(), render_manager->height());
};

void Camera::FixedUpdate() {
  const auto render_manager = Game::game()->render_manager();

  UpdateMatrices(render_manager->width(), render_manager->height());
}

void Camera::UpdateViewMatrix() {
  view_matrix_ = glm::lookAt(position_, position_ + direction_, orientation_);
}

void Camera::UpdateMatrices(int width, int height) {
  UpdateProjectionMatrix(width, height);
  UpdateViewMatrix();
}

glm::mat4 Camera::GetMvp(const glm::mat4& model_matrix) {
  return projection_matrix_ * view_matrix_ * model_matrix;
}

void Camera::position(float x, float y, float z) {
  position_ = glm::vec3(x, y, z);
}

void Camera::direction(float x, float y, float z) {
  direction_ = glm::vec3(x, y, z);
}

void Camera::orientation(float x, float y, float z) {
  orientation_ = glm::vec3(x, y, z);
}

void Camera::position(const glm::vec3& position) { position_ = position; }

void Camera::direction(const glm::vec3& direction) { direction_ = direction; }

void Camera::orientation(const glm::vec3& orientation) {
  orientation_ = orientation;
}

void Camera::nearest_point(float nearest_point) noexcept {
  nearest_point_ = nearest_point;
}

void Camera::farthest_point(float farthest_point) noexcept {
  farthest_point_ = farthest_point;
}

const glm::vec3 Camera::position() const noexcept { return position_; }

const glm::vec3 Camera::direction() const noexcept { return direction_; }

const glm::vec3 Camera::orientation() const noexcept { return orientation_; }

const float Camera::nearest_point() const noexcept { return nearest_point_; }

const float Camera::farthest_point() const noexcept { return farthest_point_; }
}  // namespace koma
