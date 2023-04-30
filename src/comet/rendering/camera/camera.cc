// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera.h"

#include "comet/event/window_event.h"
#include "comet/math/geometry.h"
#include "comet/math/quaternion.h"
#include "comet/rendering/rendering_utils.h"

namespace comet {
namespace rendering {
Camera::Camera() { Reset(); }

void Camera::Translate(const math::Vec3& translation) {
  position_.x += translation.x;
  position_.y += translation.y;
  position_.z += translation.z;
  is_view_matrix_dirty_ = true;
  is_frustum_dirty_ = true;
}

void Camera::Move(const math::Vec3& delta) {
  math::Vec3 translation{front_ * delta.z + right_ * delta.x + up_ * delta.y};
  Translate(translation);
}

void Camera::Rotate(const math::Vec2& delta) {
  auto rotation{GetRotation(delta)};
  Rotate(rotation);
}

void Camera::Rotate(const math::Quat& rotation) {
  SetRotation(rotation * rotation_);
}

void Camera::Orbit(const math::Vec2& delta) {
  auto pivot{GetCenterPivotPoint()};
  auto rotation(GetRotation(delta));
  Rotate(rotation);
  position_ = pivot + (rotation * (position_ - pivot));
}

void Camera::Reset() {
  // TODO(m4jr0): Focus on default primitive.
  SetPosition({0.0f, 7.5f, 30.0f});
  SetRotation({1.0f, 0.0f, 0.0f, 0.0f});  // No rotation.
}

void Camera::SetRotation(const math::Quat& rotation) {
  rotation_ = math::GetNormalizedCopy(rotation);
  front_ = rotation_ * -kWorldFront_;
  right_ = rotation_ * kWorldRight_;
  math::Normalize(front_);
  math::Normalize(right_);
  up_ = math::Cross(right_, front_);
  math::Normalize(up_);
  is_view_matrix_dirty_ = true;
  is_frustum_dirty_ = true;
}

void Camera::SetWidth(rendering::WindowSize width) {
  width_ = width;
  is_projection_matrix_dirty_ = true;
  is_frustum_dirty_ = true;
}

void Camera::SetHeight(rendering::WindowSize height) {
  height_ = height;
  is_projection_matrix_dirty_ = true;
  is_frustum_dirty_ = true;
}

void Camera::SetSize(rendering::WindowSize width,
                     rendering::WindowSize height) {
  width_ = width;
  height_ = height;
  is_projection_matrix_dirty_ = true;
  is_frustum_dirty_ = true;
}

void Camera::SetPosition(const math::Vec3& position) {
  position_ = position;
  is_view_matrix_dirty_ = true;
}

const math::Vec3& Camera::GetPosition() const noexcept { return position_; }

const math::Vec3& Camera::GetView() const noexcept { return front_; }

const math::Vec3& Camera::GetUp() const noexcept { return up_; }

const math::Vec3& Camera::GetRight() const noexcept { return right_; }

f32 Camera::GetNearestPoint() const noexcept { return z_near_; }

f32 comet::rendering::Camera::GetFarthestPoint() const noexcept {
  return z_far_;
}

f32 Camera::GetFov() const noexcept { return fov_; }

f32 Camera::GetFovInRadians() const noexcept {
  return math::ConvertToRadians(GetFov());
}

f32 Camera::GetRatio() const {
  if (height_ == 0) {
    return 0;
  }

  return static_cast<f32>(width_) / static_cast<f32>(height_);
}

const math::Mat4& Camera::GetProjectionMatrix() {
  if (is_projection_matrix_dirty_) {
    UpdateProjectionMatrix();
    is_projection_matrix_dirty_ = false;
  }

  return projection_matrix_;
}

rendering::WindowSize Camera::GetWidth() const noexcept { return width_; }

rendering::WindowSize Camera::GetHeight() const noexcept { return height_; }

const math::Mat4& Camera::GetViewMatrix() {
  if (is_view_matrix_dirty_) {
    UpdateViewMatrix();
    is_view_matrix_dirty_ = false;
  }

  return view_matrix_;
}

const Frustum& Camera::GetFrustum() {
  if (is_frustum_dirty_) {
    UpdateFrustum();
    is_frustum_dirty_ = false;
  }

  return frustum_;
}

void Camera::UpdateViewMatrix() {
  view_matrix_ = LookAt(position_, position_ + front_, up_);
}

void Camera::UpdateProjectionMatrix() {
  projection_matrix_ =
      GenerateProjectionMatrix(GetFovInRadians(), GetRatio(), z_near_, z_far_);
}

void Camera::UpdateFrustum() {
  const auto half_vertical{z_far_ * math::Tan(GetFovInRadians() * 0.5f)};
  const auto half_horizontal{half_vertical * GetRatio()};
  const auto z_far_vec{z_far_ * front_};

  frustum_.SetNear({position_ + z_near_ * front_, front_});
  frustum_.SetFar({position_ + z_far_vec, -front_});
  frustum_.SetLeft(
      {position_, math::Cross(z_far_vec - right_ * half_horizontal, up_)});
  frustum_.SetRight(
      {position_, math::Cross(up_, z_far_vec + right_ * half_horizontal)});
  frustum_.SetTop(
      {position_, math::Cross(z_far_vec + up_ * half_vertical, right_)});
  frustum_.SetBottom(
      {position_, math::Cross(right_, z_far_vec - up_ * half_vertical)});
}

math::Vec3 Camera::GetCenterPivotPoint() {
  const auto& ray_direction{front_};
  const auto& ray_origin{position_};
  // Find point where the ray intersects the XY plane.
  const auto t{-ray_origin.z / ray_direction.z};
  const auto intersection_point{ray_origin + ray_direction * t};
  return intersection_point;
}

math::Quat Camera::GetRotation(const math::Vec2& delta) {
  auto yaw_rotation{math::GetQuaternionRotation(delta.x, kWorldUp_)};
  auto pitch_rotation{math::GetQuaternionRotation(delta.y, right_)};
  auto rotation{yaw_rotation * pitch_rotation};
  return rotation;
}
}  // namespace rendering
}  // namespace comet
