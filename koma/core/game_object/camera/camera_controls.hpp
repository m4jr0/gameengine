// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_CAMERA_CAMERA_CONTROLS_HPP_
#define KOMA_CORE_GAME_OBJECT_CAMERA_CAMERA_CONTROLS_HPP_

#include "core/game_object/component.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace koma {
class CameraControls : public Component {
 public:
  virtual void Update() override;

 private:
  glm::vec3 position_ = glm::vec3(0, 0, 5);

  float horizontal_angle_ = 3.14f;
  float vertical_angle_ = 0.0f;
  float speed_ = 0.01f;
  float mouse_speed_ = 0.005f;
};
}  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_CAMERA_CAMERA_CONTROLS_HPP_
