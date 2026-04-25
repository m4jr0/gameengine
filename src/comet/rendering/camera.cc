// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "camera.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/math/geometry.h"
#include "comet/math/quaternion.h"
#include "comet/rendering/type/camera.h"
#include "comet/rendering/type/common.h"
#include "comet/rendering/utils/camera_utils.h"

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

void Camera::Rotate(const math::Vec2& delta) { Rotate(GetRotation(delta)); }

void Camera::Rotate(const math::Quat& rotation) {
  SetRotation(rotation * rotation_);
}

void Camera::Orbit(const math::Vec2& delta) {
  const auto pivot{GetCenterPivotPoint()};
  const auto rotation{GetRotation(delta)};
  Rotate(rotation);
  position_ = pivot + (rotation * (position_ - pivot));
  is_view_matrix_dirty_ = true;
  is_frustum_dirty_ = true;
}

void Camera::Reset() {
  // TODO(m4jr0): Focus on default primitive.
  constexpr math::Vec3 kDefaultPosition{4.491f, 1.285f, 1.995f};
  constexpr math::Quat kDefaultRotation{.831f, .0f, .55f, .01f};

  SetPosition(kDefaultPosition);
  SetRotation(kDefaultRotation);
}

void Camera::SetPosition(const math::Vec3& position) {
  position_ = position;
  is_view_matrix_dirty_ = true;
  is_frustum_dirty_ = true;
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

void Camera::SetWidth(WindowSize width) {
  width_ = width;
  is_projection_matrix_dirty_ = true;
  is_frustum_dirty_ = true;
}

void Camera::SetHeight(WindowSize height) {
  height_ = height;
  is_projection_matrix_dirty_ = true;
  is_frustum_dirty_ = true;
}

void Camera::SetSize(WindowSize width, WindowSize height) {
  width_ = width;
  height_ = height;
  is_projection_matrix_dirty_ = true;
  is_frustum_dirty_ = true;
}

const math::Vec3& Camera::GetPosition() const noexcept { return position_; }

const math::Vec3& Camera::GetFront() const noexcept { return front_; }

const math::Vec3& Camera::GetUp() const noexcept { return up_; }

const math::Vec3& Camera::GetRight() const noexcept { return right_; }

f32 Camera::GetNearestPoint() const noexcept { return z_near_; }

f32 Camera::GetFarthestPoint() const noexcept { return z_far_; }

f32 Camera::GetFov() const noexcept { return fov_; }

f32 Camera::GetFovInRadians() const noexcept {
  return math::ConvertToRadians(GetFov());
}

f32 Camera::GetRatio() const {
  if (height_ == 0) {
    return 1.0f;
  }

  return static_cast<f32>(width_) / static_cast<f32>(height_);
}

WindowSize Camera::GetWidth() const noexcept { return width_; }

WindowSize Camera::GetHeight() const noexcept { return height_; }

const math::Mat4& Camera::GetProjectionMatrix() {
  if (is_projection_matrix_dirty_) {
    UpdateProjectionMatrix();
    is_projection_matrix_dirty_ = false;
  }

  return projection_matrix_;
}

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

void Camera::PopulateData(RenderCameraData& data) {
  data.projection_matrix = GetProjectionMatrix();
  data.view_matrix = GetViewMatrix();
  data.view_position = GetPosition();
  data.front = GetFront();
  data.up = GetUp();
  data.right = GetRight();
  data.near_plane = GetNearestPoint();
  data.far_plane = GetFarthestPoint();
  data.fov_y_radians = GetFovInRadians();
  data.aspect_ratio = GetRatio();
}

void Camera::UpdateViewMatrix() {
  view_matrix_ = LookAt(position_, position_ + front_, up_);
}

void Camera::UpdateProjectionMatrix() {
  projection_matrix_ =
      GeneratePerspectiveMatrix(GetFovInRadians(), GetRatio(), z_near_, z_far_,
                                ClipSpaceDepthRange::ZeroToOne);
}

void Camera::UpdateFrustum() {
  const auto half_vertical{z_far_ * math::Tan(GetFovInRadians() * .5f)};
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
  return ray_origin + ray_direction * t;
}

math::Quat Camera::GetRotation(const math::Vec2& delta) {
  const auto yaw_rotation{math::GetQuaternionRotation(delta.x, kWorldUp_)};
  const auto pitch_rotation{math::GetQuaternionRotation(delta.y, right_)};
  return yaw_rotation * pitch_rotation;
}
}  // namespace rendering
}  // namespace comet