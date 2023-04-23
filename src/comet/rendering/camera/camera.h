// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_CAMERA_CAMERA_H_
#define COMET_COMET_RENDERING_CAMERA_CAMERA_H_

#include "comet_precompile.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include "comet/event/event.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
class Camera {
 public:
  Camera();
  Camera(const Camera&) = delete;
  Camera(Camera&&) = delete;
  Camera& operator=(const Camera&) = delete;
  Camera& operator=(Camera&&) = delete;
  virtual ~Camera() = default;

  void Translate(const glm::vec3& translation);
  void Move(const glm::vec3& delta);
  void Rotate(const glm::vec2& delta);
  void Rotate(const glm::quat& rotation);
  void Orbit(const glm::vec2& delta);
  void Reset();
  void SetPosition(const glm::vec3& position);
  void SetRotation(const glm::quat& rotation);
  const glm::vec3& GetPosition() const noexcept;
  const glm::vec3& GetView() const noexcept;
  const glm::vec3& GetUp() const noexcept;
  const glm::vec3& GetRight() const noexcept;
  f32 GetNearestPoint() const noexcept;
  f32 GetFarthestPoint() const noexcept;
  f32 GetFov() const noexcept;
  f32 GetFovInRadiants() const noexcept;
  f32 GetRatio() const;
  rendering::WindowSize GetWidth() const noexcept;
  rendering::WindowSize GetHeight() const noexcept;
  const glm::mat4& GetProjectionMatrix();
  const glm::mat4& GetViewMatrix();

 private:
  void UpdateViewMatrix();
  void UpdateProjectionMatrix();
  glm::vec3 GetCenterPivotPoint();
  glm::quat GetRotation(const glm::vec2& delta);
  void OnEvent(const event::Event& event);

  static constexpr glm::vec3 kWorldUp_{0.0f, 1.0f, 0.0f};
  static constexpr glm::vec3 kWorldRight_{1.0f, 0.0f, 0.0f};
  static constexpr glm::vec3 kWorldFront_{0.0f, 0.0f, -1.0f};
  bool is_projection_matrix_dirty_{true};
  bool is_view_matrix_dirty_{true};
  f32 nearest_point_{0.1f};
  f32 farthest_point_{100.0f};
  f32 fov_{45.0f};
  rendering::WindowSize width_{0};
  rendering::WindowSize height_{0};
  glm::vec3 position_{};
  glm::quat rotation_{};

  glm::vec3 front_{};
  glm::vec3 up_{};
  glm::vec3 right_{};
  glm::mat4 projection_matrix_{1.0f};  // Identity matrix.
  glm::mat4 view_matrix_{1.0f};        // Identity matrix.
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_CAMERA_CAMERA_H_
