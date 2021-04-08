// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera_controls.h"

#include "comet/core/engine.h"
#include "comet/game_object/camera/camera.h"
#include "comet/input/input_manager.h"
#include "comet/rendering/rendering_manager.h"
#include "comet/time/time_manager.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace game_object {
void CameraControls::Update() {
  const auto time_manager = core::Engine::engine()->time_manager();
  const auto rendering_manager = core::Engine::engine()->rendering_manager();
  auto input_manager = core::Engine::engine()->input_manager();

  const auto time_delta = time_manager->time_delta();
  const auto current_mouse_pos = input_manager->GetMousePosition();
  const auto half_width = rendering_manager->window()->width() / 2;
  const auto half_height = rendering_manager->window()->height() / 2;

  horizontal_angle_ = mouse_speed_ * float(half_width - current_mouse_pos.x);
  vertical_angle_ = mouse_speed_ * float(half_height - current_mouse_pos.y);

  const auto direction = glm::vec3(
      cos(vertical_angle_) * sin(horizontal_angle_), sin(vertical_angle_),
      cos(vertical_angle_) * cos(horizontal_angle_));

  const auto right = glm::vec3(sin(horizontal_angle_ - 3.14f / 2.0f), 0,
                               cos(horizontal_angle_ - 3.14f / 2.0f));

  const auto up = glm::cross(right, direction);

  if (input_manager->IsKeyDown(input::KeyCode::W)) {
    position_ += direction * static_cast<float>(time_delta) * speed_;
  }

  if (input_manager->IsKeyDown(input::KeyCode::S)) {
    position_ -= direction * static_cast<float>(time_delta) * speed_;
  }

  if (input_manager->IsKeyDown(input::KeyCode::D)) {
    position_ += right * static_cast<float>(time_delta) * speed_;
  }

  if (input_manager->IsKeyDown(input::KeyCode::A)) {
    position_ -= right * static_cast<float>(time_delta) * speed_;
  }

  auto main_camera = core::Engine::engine()->main_camera();

  main_camera->position(position_);
  main_camera->direction(direction);
  main_camera->orientation(up);
}
}  // namespace game_object
}  // namespace comet
