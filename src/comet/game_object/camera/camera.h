// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GAME_OBJECT_CAMERA_CAMERA_H_
#define COMET_COMET_GAME_OBJECT_CAMERA_CAMERA_H_

#include "comet/game_object/component.h"
#include "comet_precompile.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace comet {
namespace game_object {
class Camera : public Component {
 public:
  Camera() = default;
  Camera(const Camera&);
  Camera(Camera&&) noexcept;
  Camera& operator=(const Camera&);
  Camera& operator=(Camera&&) noexcept;
  virtual ~Camera() = default;

  virtual void Initialize() override;
  virtual void FixedUpdate() override;
  virtual void UpdateMatrices(int, int);
  virtual void UpdateProjectionMatrix(int width, int height) = 0;
  virtual void UpdateViewMatrix();
  virtual glm::mat4 GetMvp(const glm::mat4& model_matrix) final;

  const glm::vec3& GetPosition() const noexcept;
  const glm::vec3& GetDirection() const noexcept;
  const glm::vec3& GetOrientation() const noexcept;
  float GetNearestPoint() const noexcept;
  float GetFarthestPoint() const noexcept;
  void SetPosition(float x, float y, float z);
  void SetDirection(float x, float y, float z);
  void SetOrientation(float x, float y, float z);
  void SetPosition(const glm::vec3& position);
  void SetDirection(const glm::vec3& direction);
  void SetOrientation(const glm::vec3& orientation);
  void SetNearestPoint(float nearest_point) noexcept;
  void SetFarthestPoint(float farthest_point) noexcept;

 protected:
  glm::vec3 position_ = glm::vec3(0, 0, 0);
  glm::vec3 direction_ = glm::vec3(0, 0, 0);
  glm::vec3 orientation_ = glm::vec3(0, 1, 0);

  float nearest_point_ = 0.1f;
  float farthest_point_ = 100.0f;

  glm::mat4 projection_matrix_{};
  glm::mat4 view_matrix_{};
  glm::mat4 model_matrix_{};
};
}  // namespace game_object
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_CAMERA_CAMERA_H_
