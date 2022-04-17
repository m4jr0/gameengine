// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera.h"

#include "comet/core/engine.h"
#include "comet/rendering/rendering_manager.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace game_object {
Camera::Camera(const Camera& other)
    : Component(other),
      position_(other.position_),
      direction_(other.direction_),
      orientation_(other.orientation_),
      nearest_point_(other.nearest_point_),
      farthest_point_(other.farthest_point_),
      projection_matrix_(other.projection_matrix_),
      view_matrix_(other.view_matrix_),
      model_matrix_(other.model_matrix_) {}

Camera::Camera(Camera&& other) noexcept
    : Component(std::move(other)),
      position_(std::move(other.position_)),
      direction_(std::move(other.direction_)),
      orientation_(std::move(other.orientation_)),
      nearest_point_(std::move(other.nearest_point_)),
      farthest_point_(std::move(other.farthest_point_)),
      projection_matrix_(std::move(other.projection_matrix_)),
      view_matrix_(std::move(other.view_matrix_)),
      model_matrix_(std::move(other.model_matrix_)) {}

Camera& Camera::operator=(const Camera& other) {
  if (this == &other) {
    return *this;
  }

  Component::operator=(other);

  position_ = other.position_;
  direction_ = other.direction_;
  orientation_ = other.orientation_;

  nearest_point_ = other.nearest_point_;
  farthest_point_ = other.farthest_point_;

  projection_matrix_ = other.projection_matrix_;
  view_matrix_ = other.view_matrix_;
  model_matrix_ = other.model_matrix_;

  return *this;
}

Camera& Camera::operator=(Camera&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Component::operator=(std::move(other));

  position_ = std::move(other.position_);
  direction_ = std::move(other.direction_);
  orientation_ = std::move(other.orientation_);

  nearest_point_ = std::move(other.nearest_point_);
  farthest_point_ = std::move(other.farthest_point_);

  projection_matrix_ = std::move(other.projection_matrix_);
  view_matrix_ = std::move(other.view_matrix_);
  model_matrix_ = std::move(other.model_matrix_);

  return *this;
}

void Camera::Initialize() {
  Component::Initialize();

  const auto& rendering_manager =
      core::Engine::GetEngine().GetRenderingManager();

  UpdateMatrices(rendering_manager.GetWindow()->GetWidth(),
                 rendering_manager.GetWindow()->GetHeight());
};

void Camera::FixedUpdate() {
  const auto& rendering_manager =
      core::Engine::GetEngine().GetRenderingManager();

  UpdateMatrices(rendering_manager.GetWindow()->GetWidth(),
                 rendering_manager.GetWindow()->GetHeight());
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

const glm::vec3& Camera::GetPosition() const noexcept { return position_; }

const glm::vec3& Camera::GetDirection() const noexcept { return direction_; }

const glm::vec3& Camera::GetOrientation() const noexcept {
  return orientation_;
}

float Camera::GetNearestPoint() const noexcept { return nearest_point_; }

float Camera::GetFarthestPoint() const noexcept { return farthest_point_; }

void Camera::SetPosition(float x, float y, float z) {
  position_ = glm::vec3(x, y, z);
}

void Camera::SetDirection(float x, float y, float z) {
  direction_ = glm::vec3(x, y, z);
}

void Camera::SetOrientation(float x, float y, float z) {
  orientation_ = glm::vec3(x, y, z);
}

void Camera::SetPosition(const glm::vec3& position) { position_ = position; }

void Camera::SetDirection(const glm::vec3& direction) {
  direction_ = direction;
}

void Camera::SetOrientation(const glm::vec3& orientation) {
  orientation_ = orientation;
}

void Camera::SetNearestPoint(float nearest_point) noexcept {
  nearest_point_ = nearest_point;
}

void Camera::SetFarthestPoint(float farthest_point) noexcept {
  farthest_point_ = farthest_point;
}
}  // namespace game_object
}  // namespace comet
