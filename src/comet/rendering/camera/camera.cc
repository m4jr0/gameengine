// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera.h"

#include "comet/core/engine.h"
#include "comet/event/window_event.h"
#include "comet/utils/math/math_commons.h"

namespace comet {
namespace rendering {
Camera::Camera()
    : width_{Engine::Get().GetRenderingManager().GetWindow()->GetWidth()},
      height_{Engine::Get().GetRenderingManager().GetWindow()->GetHeight()} {
  Reset();

  Engine::Get().GetEventManager().Register(
      COMET_EVENT_BIND_FUNCTION(Camera::OnEvent),
      event::WindowResizeEvent::kStaticType_);
}

void Camera::Translate(const glm::vec3& translation) {
  position_.x += translation.x;
  position_.y += translation.y;
  position_.z += translation.z;
  is_view_matrix_dirty_ = true;
}

void Camera::Move(const glm::vec3& delta) {
  glm::vec3 translation{front_ * delta.z + right_ * delta.x + up_ * delta.y};
  Translate(translation);
}

void Camera::Rotate(const glm::vec2& delta) {
  auto rotation{GetRotation(delta)};
  Rotate(rotation);
}

void Camera::Rotate(const glm::quat& rotation) {
  SetRotation(rotation * rotation_);
}

void Camera::Orbit(const glm::vec2& delta) {
  auto pivot{GetCenterPivotPoint()};
  auto rotation(GetRotation(delta));
  Rotate(rotation);
  position_ = pivot + (rotation * (position_ - pivot));
}

void Camera::Reset() {
  // TODO(m4jr0): Focus on default primitive.
  SetPosition({0.0f, 7.5f, 21.0f});
  SetRotation({0.0f, 0.0f, 0.0f, 1.0f});  // No rotation.
}

void Camera::SetRotation(const glm::quat& rotation) {
  rotation_ = glm::normalize(rotation);
  front_ = glm::normalize(rotation_ * kWorldFront_);
  right_ = glm::normalize(rotation_ * kWorldRight_);
  up_ = glm::normalize(glm::cross(front_, right_));
  is_view_matrix_dirty_ = true;
}

void Camera::SetPosition(const glm::vec3& position) {
  position_ = position;
  is_view_matrix_dirty_ = true;
}

const glm::vec3& Camera::GetPosition() const noexcept { return position_; }

const glm::vec3& Camera::GetView() const noexcept { return front_; }

const glm::vec3& Camera::GetUp() const noexcept { return up_; }

const glm::vec3& Camera::GetRight() const noexcept { return right_; }

f32 Camera::GetNearestPoint() const noexcept { return nearest_point_; }

f32 comet::rendering::Camera::GetFarthestPoint() const noexcept {
  return farthest_point_;
}

f32 Camera::GetFov() const noexcept { return fov_; }

f32 Camera::GetFovInRadiants() const noexcept {
  return utils::math::ConvertToRadiants(GetFov());
}

f32 Camera::GetRatio() const {
  if (height_ == 0) {
    return 0;
  }

  return static_cast<f32>(width_) / static_cast<f32>(height_);
}

const glm::mat4& Camera::GetProjectionMatrix() {
  if (is_projection_matrix_dirty_) {
    UpdateProjectionMatrix();
    is_projection_matrix_dirty_ = false;
  }

  return projection_matrix_;
}

rendering::WindowSize Camera::GetWidth() const noexcept { return width_; }

rendering::WindowSize Camera::GetHeight() const noexcept { return height_; }

const glm::mat4& Camera::GetViewMatrix() {
  if (is_view_matrix_dirty_) {
    UpdateViewMatrix();
    is_view_matrix_dirty_ = false;
  }

  return view_matrix_;
}

void Camera::UpdateViewMatrix() {
  view_matrix_ = glm::lookAt(position_, position_ + front_, up_);
}

void Camera::UpdateProjectionMatrix() {
  projection_matrix_ = glm::perspective(GetFovInRadiants(), GetRatio(),
                                        nearest_point_, farthest_point_);
}

glm::vec3 Camera::GetCenterPivotPoint() {
  const auto& ray_direction{front_};
  const auto& ray_origin{position_};
  // Find point where the ray intersects the XY plane.
  const auto t{-ray_origin.z / ray_direction.z};
  const auto intersection_point{ray_origin + ray_direction * t};
  return intersection_point;
}

glm::quat Camera::GetRotation(const glm::vec2& delta) {
  auto yaw_rotation{glm::angleAxis(delta.x, kWorldUp_)};
  auto pitch_rotation{glm::angleAxis(delta.y, right_)};
  auto rotation{yaw_rotation * pitch_rotation};
  return rotation;
}

void Camera::OnEvent(const event::Event& event) {
  if (event.GetType() != event::WindowResizeEvent::kStaticType_) {
    return;
  }

  const auto& window_resize_event{
      static_cast<const event::WindowResizeEvent&>(event)};
  width_ = window_resize_event.GetWidth();
  height_ = window_resize_event.GetHeight();
  is_projection_matrix_dirty_ = true;
}
}  // namespace rendering
}  // namespace comet
