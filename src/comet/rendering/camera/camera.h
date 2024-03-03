// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_CAMERA_CAMERA_H_
#define COMET_COMET_RENDERING_CAMERA_CAMERA_H_

#include "comet_precompile.h"

#include "comet/event/event.h"
#include "comet/math/matrix.h"
#include "comet/math/quaternion.h"
#include "comet/math/vector.h"
#include "comet/rendering/camera/frustum.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
class Camera {
 public:
  Camera();
  Camera(const Camera&) = default;
  Camera(Camera&&) = default;
  Camera& operator=(const Camera&) = default;
  Camera& operator=(Camera&&) = default;
  virtual ~Camera() = default;

  void Translate(const math::Vec3& translation);
  void Move(const math::Vec3& delta);
  void Rotate(const math::Vec2& delta);
  void Rotate(const math::Quat& rotation);
  void Orbit(const math::Vec2& delta);
  void Reset();
  void SetPosition(const math::Vec3& position);
  void SetRotation(const math::Quat& rotation);
  void SetWidth(rendering::WindowSize width);
  void SetHeight(rendering::WindowSize height);
  void SetSize(rendering::WindowSize width, rendering::WindowSize height);
  const math::Vec3& GetPosition() const noexcept;
  const math::Vec3& GetView() const noexcept;
  const math::Vec3& GetUp() const noexcept;
  const math::Vec3& GetRight() const noexcept;
  f32 GetNearestPoint() const noexcept;
  f32 GetFarthestPoint() const noexcept;
  f32 GetFov() const noexcept;
  f32 GetFovInRadians() const noexcept;
  f32 GetRatio() const;
  rendering::WindowSize GetWidth() const noexcept;
  rendering::WindowSize GetHeight() const noexcept;
  const math::Mat4& GetProjectionMatrix();
  const math::Mat4& GetViewMatrix();
  const Frustum& GetFrustum();

 private:
  void UpdateViewMatrix();
  void UpdateProjectionMatrix();
  void UpdateFrustum();
  math::Vec3 GetCenterPivotPoint();
  math::Quat GetRotation(const math::Vec2& delta);

  static constexpr math::Vec3 kWorldUp_{0.0f, 1.0f, 0.0f};
  static constexpr math::Vec3 kWorldRight_{1.0f, 0.0f, 0.0f};
  static constexpr math::Vec3 kWorldFront_{0.0f, 0.0f, 1.0f};
  bool is_projection_matrix_dirty_{true};
  bool is_view_matrix_dirty_{true};
  bool is_frustum_dirty_{true};
  f32 z_near_{0.1f};
  f32 z_far_{1000.0f};
  f32 fov_{45.0f};
  rendering::WindowSize width_{0};
  rendering::WindowSize height_{0};
  math::Vec3 position_{};
  math::Quat rotation_{};
  math::Vec3 front_{};
  math::Vec3 up_{};
  math::Vec3 right_{};
  math::Mat4 projection_matrix_{1.0f};  // Identity matrix.
  math::Mat4 view_matrix_{1.0f};        // Identity matrix.
  Frustum frustum_{};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_CAMERA_CAMERA_H_
