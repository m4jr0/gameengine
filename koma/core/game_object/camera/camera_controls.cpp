// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allow debugging memory leaks.
#include "../../../debug.hpp"

#include "camera_controls.hpp"

#include "../../locator/locator.hpp"
#include "../../game_object/camera/camera.hpp"
#include "../../rendering/rendering_manager.hpp"
#include "../../time/time_manager.hpp"

namespace koma {
void CameraControls::Update() {
  TimeManager time_manager = Locator::time_manager();
  RenderingManager rendering_manager = Locator::rendering_manager();

  double time_delta = time_manager.time_delta();
  double current_mouse_y_pos, current_mouse_x_pos;
  GLFWwindow *window = const_cast<GLFWwindow *>(rendering_manager.window());

  glfwGetCursorPos(
    window,
    &current_mouse_x_pos,
    &current_mouse_y_pos
  );

  float middle_x = rendering_manager.width() / 2;
  float middle_y = rendering_manager.height() / 2;

  glfwSetCursorPos(
    window,
    middle_x,
    middle_y
  );

  this->horizontal_angle_ += this->mouse_speed_ *
    float(middle_x - current_mouse_x_pos);

  this->vertical_angle_ += this->mouse_speed_ *
    float(middle_y - current_mouse_y_pos);

  glm::vec3 direction = glm::vec3(
    cos(this->vertical_angle_) * sin(this->horizontal_angle_),
    sin(this->vertical_angle_),
    cos(this->vertical_angle_) * cos(this->horizontal_angle_)
  );

  glm::vec3 right = glm::vec3(
    sin(this->horizontal_angle_ - 3.14f / 2.0f),
    0,
    cos(this->horizontal_angle_ - 3.14f / 2.0f)
  );

  glm::vec3 up = glm::cross(right, direction);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    this->position_ += direction * (float)time_delta * this->speed_;
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    this->position_ -= direction * (float)time_delta * this->speed_;
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    this->position_ += right * (float)time_delta * this->speed_;
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    this->position_ -= right * (float)time_delta * this->speed_;
  }

  auto main_camera = Locator::main_camera();

  main_camera->position(this->position_);
  main_camera->direction(direction);
  main_camera->orientation(up);
}
};  // namespace koma
