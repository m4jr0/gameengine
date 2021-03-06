// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera_controls.hpp"

#include "core/game.hpp"
#include "core/game_object/camera/camera.hpp"
#include "core/input/input_manager.hpp"
#include "core/render/render_manager.hpp"
#include "core/time/time_manager.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

namespace koma {
void CameraControls::Update() {
  const auto time_manager = Game::game()->time_manager();
  const auto render_manager = Game::game()->render_manager();
  auto input_manager = Game::game()->input_manager();

  const auto time_delta = time_manager->time_delta();
  const auto current_mouse_pos = input_manager->GetMousePosition();
  const auto half_width = render_manager->width() / 2;
  const auto half_height = render_manager->height() / 2;

  horizontal_angle_ = mouse_speed_ * float(half_width - current_mouse_pos.x);
  vertical_angle_ = mouse_speed_ * float(half_height - current_mouse_pos.y);

  const auto direction = glm::vec3(
      cos(vertical_angle_) * sin(horizontal_angle_), sin(vertical_angle_),
      cos(vertical_angle_) * cos(horizontal_angle_));

  const auto right = glm::vec3(sin(horizontal_angle_ - 3.14f / 2.0f), 0,
                               cos(horizontal_angle_ - 3.14f / 2.0f));

  const auto up = glm::cross(right, direction);

  if (input_manager->IsKeyDown(KeyCode::W)) {
    position_ += direction * static_cast<float>(time_delta) * speed_;
  }

  if (input_manager->IsKeyDown(KeyCode::S)) {
    position_ -= direction * static_cast<float>(time_delta) * speed_;
  }

  if (input_manager->IsKeyDown(KeyCode::D)) {
    position_ += right * static_cast<float>(time_delta) * speed_;
  }

  if (input_manager->IsKeyDown(KeyCode::A)) {
    position_ -= right * static_cast<float>(time_delta) * speed_;
  }

  auto main_camera = Game::game()->main_camera();

  main_camera->position(position_);
  main_camera->direction(direction);
  main_camera->orientation(up);
}
}  // namespace koma
