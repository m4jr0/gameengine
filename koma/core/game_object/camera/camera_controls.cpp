// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera_controls.hpp"

#include "core/game_object/camera/camera.hpp"
#include "core/input/input_manager.hpp"
#include "core/locator/locator.hpp"
#include "core/render/render_manager.hpp"
#include "core/time/time_manager.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

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

  horizontal_angle_ += mouse_speed_ * float(half_width - current_mouse_pos.x);

  vertical_angle_ += mouse_speed_ * float(half_height - current_mouse_pos.y);

  glm::vec3 direction = glm::vec3(
      cos(vertical_angle_) * sin(horizontal_angle_), sin(vertical_angle_),
      cos(vertical_angle_) * cos(horizontal_angle_));

  glm::vec3 right = glm::vec3(sin(horizontal_angle_ - 3.14f / 2.0f), 0,
                              cos(horizontal_angle_ - 3.14f / 2.0f));

  glm::vec3 up = glm::cross(right, direction);

  if (input_manager.GetKeyDown(KeyCode::W)) {
    position_ += direction * (float)time_delta * speed_;
  }

  if (input_manager.GetKeyDown(KeyCode::S)) {
    position_ -= direction * (float)time_delta * speed_;
  }

  if (input_manager.GetKeyDown(KeyCode::D)) {
    position_ += right * (float)time_delta * speed_;
  }

  if (input_manager.GetKeyDown(KeyCode::A)) {
    position_ -= right * (float)time_delta * speed_;
  }

  auto main_camera = Locator::main_camera();

  main_camera->position(position_);
  main_camera->direction(direction);
  main_camera->orientation(up);
}
}  // namespace koma
