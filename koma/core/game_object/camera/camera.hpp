// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_CAMERA_CAMERA_HPP_
#define KOMA_CORE_GAME_OBJECT_CAMERA_CAMERA_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../locator/locator.hpp"
#include "../component.hpp"

namespace koma {
class Camera : public Component {
 public:
  virtual void UpdateMatrices(GLuint, GLuint);
  virtual void UpdateProjectionMatrix(GLuint, GLuint) = 0;
  virtual void UpdateViewMatrix();
  virtual glm::mat4 GetMvp(glm::mat4) final;

  virtual void Initialize() override;
  virtual void FixedUpdate() override;

  void position(float, float, float);
  void look_at(float, float, float);
  void orientation(float, float, float);

  void position(glm::vec3);
  void look_at(glm::vec3);
  void orientation(glm::vec3);

  void nearest_point(float);
  void farthest_point(float);

  const glm::vec3 position() const noexcept;
  const glm::vec3 look_at() const noexcept;
  const glm::vec3 orientation() const noexcept;

  const float nearest_point() const noexcept;
  const float farthest_point() const noexcept;

 protected:
  glm::vec3 position_;
  glm::vec3 look_at_;
  glm::vec3 orientation_;

  float nearest_point_;
  float farthest_point_;

  glm::mat4 projection_matrix_;
  glm::mat4 view_matrix_;
  glm::mat4 model_matrix_;
};
};  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_CAMERA_CAMERA_HPP_