// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera_controls.hpp"

#include <core/input/input_manager.hpp>
#include <core/locator/locator.hpp>
#include <core/game_object/camera/camera.hpp>
#include <core/render/render_manager.hpp>
#include <core/time/time_manager.hpp>

// Allow debugging memory leaks.
#include <debug.hpp>

namespace koma {
void CameraControls::Update() {
  TimeManager time_manager = Locator::time_manager();
  RenderManager render_manager = Locator::render_manager();
  InputManager input_manager = Locator::input_manager();

  double time_delta = time_manager.time_delta();
  glm::vec2 current_mouse_pos = input_manager.GetMousePosition();

  float half_width = render_manager.width() / 2;
  float half_height = render_manager.height() / 2;

  input_manager.SetMousePosition(half_width, half_height);

  this->horizontal_angle_ += this->mouse_speed_ *
    float(half_width - current_mouse_pos.x);

  this->vertical_angle_ += this->mouse_speed_ *
    float(half_height - current_mouse_pos.y);

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

  if (input_manager.GetKeyDown(KeyCode::W)) {
    this->position_ += direction * (float)time_delta * this->speed_;
  }

  if (input_manager.GetKeyDown(KeyCode::S)) {
    this->position_ -= direction * (float)time_delta * this->speed_;
  }

  if (input_manager.GetKeyDown(KeyCode::D)) {
    this->position_ += right * (float)time_delta * this->speed_;
  }

  if (input_manager.GetKeyDown(KeyCode::A)) {
    this->position_ -= right * (float)time_delta * this->speed_;
  }

  auto main_camera = Locator::main_camera();

  main_camera->position(this->position_);
  main_camera->direction(direction);
  main_camera->orientation(up);
}
};  // namespace koma
