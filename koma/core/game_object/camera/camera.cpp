// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allow debugging memory leaks.
#include "../../../debug.hpp"

#include "camera.hpp"

#include "../../locator/locator.hpp"
#include "../../rendering/rendering_manager.hpp"

namespace koma {
void Camera::UpdateViewMatrix() {
  this->view_matrix_ = glm::lookAt(
    this->position_,
    this->look_at_,
    this->orientation_
  );
}

void Camera::UpdateMatrices(int width, int height) {
  this->UpdateProjectionMatrix(width, height);
  this->UpdateViewMatrix();
}

glm::mat4 Camera::GetMvp(glm::mat4 model_matrix) {
  return this->projection_matrix_ * this->view_matrix_ * model_matrix;
}

void Camera::Initialize() {
  RenderingManager rendering_manager = Locator::rendering_manager();

  this->UpdateMatrices(
    rendering_manager.width(), rendering_manager.height()
  );
};

void Camera::FixedUpdate() {
  RenderingManager rendering_manager = Locator::rendering_manager();

  this->UpdateMatrices(
    rendering_manager.width(), rendering_manager.height()
  );
}

void Camera::position(float x, float y, float z) {
  this->position_ = glm::vec3(x, y, z);
}

void Camera::look_at(float x, float y, float z) {
  this->look_at_ = glm::vec3(x, y, z);
}

void Camera::orientation(float x, float y, float z) {
  this->orientation_ = glm::vec3(x, y, z);
}

void Camera::position(glm::vec3 position) {
  this->position_ = position;
}

void Camera::look_at(glm::vec3 look_at) {
  this->look_at_ = look_at;
}

void Camera::orientation(glm::vec3 orientation) {
  this->orientation_ = orientation;
}

void Camera::nearest_point(float nearest_point) noexcept {
  this->nearest_point_ = nearest_point;
}

void Camera::farthest_point(float farthest_point) noexcept {
  this->farthest_point_ = farthest_point;
}

const glm::vec3 Camera::position() const noexcept {
  return this->position_;
}

const glm::vec3 Camera::look_at() const noexcept {
  return this->look_at_;
}

const glm::vec3 Camera::orientation() const noexcept {
  return this->orientation_;
}

const float Camera::nearest_point() const noexcept {
  return this->nearest_point_;
}

const float Camera::farthest_point() const noexcept {
  return this->farthest_point_;
}
};  // namespace koma
