// Copyright 2021 m4jr0. All Rights Reserved.
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
  virtual void Initialize() override;
  virtual void FixedUpdate() override;

  virtual void UpdateMatrices(int, int);
  virtual void UpdateProjectionMatrix(int, int) = 0;
  virtual void UpdateViewMatrix();
  virtual glm::mat4 GetMvp(const glm::mat4&) final;

  void position(float, float, float);
  void direction(float, float, float);
  void orientation(float, float, float);
  void position(const glm::vec3&);
  void direction(const glm::vec3&);
  void orientation(const glm::vec3&);
  void nearest_point(float) noexcept;
  void farthest_point(float) noexcept;
  const glm::vec3 position() const noexcept;
  const glm::vec3 direction() const noexcept;
  const glm::vec3 orientation() const noexcept;
  const float nearest_point() const noexcept;
  const float farthest_point() const noexcept;

 protected:
  glm::vec3 position_ = glm::vec3(0, 0, 0);
  glm::vec3 direction_ = glm::vec3(0, 0, 0);
  glm::vec3 orientation_ = glm::vec3(0, 1, 0);

  float nearest_point_ = 0.1f;
  float farthest_point_ = 100.0f;

  glm::mat4 projection_matrix_;
  glm::mat4 view_matrix_;
  glm::mat4 model_matrix_;
};
}  // namespace game_object
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_CAMERA_CAMERA_H_
