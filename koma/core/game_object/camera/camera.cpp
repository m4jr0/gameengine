// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera.hpp"

#include <core/locator/locator.hpp>
#include <core/render/render_manager.hpp>

// Allow debugging memory leaks.
#include <debug.hpp>

namespace koma {
void Camera::UpdateViewMatrix() {
  this->view_matrix_ = glm::lookAt(
    this->position_,
    this->position_ + this->direction_,
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
  this->Component::Initialize();

  RenderManager render_manager = Locator::render_manager();

  this->UpdateMatrices(
    render_manager.width(), render_manager.height()
  );
};

void Camera::FixedUpdate() {
  RenderManager render_manager = Locator::render_manager();

  this->UpdateMatrices(
    render_manager.width(), render_manager.height()
  );
}

void Camera::position(float x, float y, float z) {
  this->position_ = glm::vec3(x, y, z);
}

void Camera::direction(float x, float y, float z) {
  this->direction_ = glm::vec3(x, y, z);
}

void Camera::orientation(float x, float y, float z) {
  this->orientation_ = glm::vec3(x, y, z);
}

void Camera::position(glm::vec3 position) {
  this->position_ = position;
}

void Camera::direction(glm::vec3 direction) {
  this->direction_ = direction;
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

const glm::vec3 Camera::direction() const noexcept {
  return this->direction_;
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
