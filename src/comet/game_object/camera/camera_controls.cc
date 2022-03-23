// Copyright 2022 m4jr0. All Rights Reserved.
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
CameraControls::CameraControls(const CameraControls& other)
    : Component(other),
      position_(other.position_),
      horizontal_angle_(other.horizontal_angle_),
      vertical_angle_(other.vertical_angle_),
      speed_(other.speed_),
      mouse_speed_(other.mouse_speed_) {}

CameraControls::CameraControls(CameraControls&& other) noexcept
    : Component(std::move(other)),
      position_(std::move(other.position_)),
      horizontal_angle_(std::move(other.horizontal_angle_)),
      vertical_angle_(std::move(other.vertical_angle_)),
      speed_(std::move(other.speed_)),
      mouse_speed_(std::move(other.mouse_speed_)) {}

CameraControls& CameraControls::operator=(const CameraControls& other) {
  if (this == &other) {
    return *this;
  }

  Component::operator=(other);
  position_ = other.position_;
  horizontal_angle_ = other.horizontal_angle_;
  vertical_angle_ = other.vertical_angle_;
  speed_ = other.speed_;
  mouse_speed_ = other.mouse_speed_;
  return *this;
}

CameraControls& CameraControls::operator=(CameraControls&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Component::operator=(std::move(other));
  position_ = std::move(other.position_);
  horizontal_angle_ = std::move(other.horizontal_angle_);
  vertical_angle_ = std::move(other.vertical_angle_);
  speed_ = std::move(other.speed_);
  mouse_speed_ = std::move(other.mouse_speed_);
  return *this;
}

std::shared_ptr<Component> CameraControls::Clone() const {
  return std::make_shared<CameraControls>(*this);
}

void CameraControls::Update() {
  const auto& time_manager = core::Engine::GetEngine().GetTimeManager();
  const auto& rendering_manager =
      core::Engine::GetEngine().GetRenderingManager();
  const auto& input_manager = core::Engine::GetEngine().GetInputManager();

  const auto time_delta = time_manager.GetTimeDelta();
  const auto current_mouse_pos = input_manager.GetMousePosition();
  const auto half_width = rendering_manager.GetWindow()->GetWidth() / 2;
  const auto half_height = rendering_manager.GetWindow()->GetHeight() / 2;

  horizontal_angle_ = mouse_speed_ * float(half_width - current_mouse_pos.x);
  vertical_angle_ = mouse_speed_ * float(half_height - current_mouse_pos.y);

  const auto direction = glm::vec3(
      cos(vertical_angle_) * sin(horizontal_angle_), sin(vertical_angle_),
      cos(vertical_angle_) * cos(horizontal_angle_));

  const auto right = glm::vec3(sin(horizontal_angle_ - 3.14f / 2.0f), 0,
                               cos(horizontal_angle_ - 3.14f / 2.0f));

  const auto up = glm::cross(right, direction);

  if (input_manager.IsKeyDown(input::KeyCode::W)) {
    position_ += direction * static_cast<float>(time_delta) * speed_;
  }

  if (input_manager.IsKeyDown(input::KeyCode::S)) {
    position_ -= direction * static_cast<float>(time_delta) * speed_;
  }

  if (input_manager.IsKeyDown(input::KeyCode::D)) {
    position_ += right * static_cast<float>(time_delta) * speed_;
  }

  if (input_manager.IsKeyDown(input::KeyCode::A)) {
    position_ -= right * static_cast<float>(time_delta) * speed_;
  }

  auto& main_camera = core::Engine::GetEngine().GetMainCamera();

  main_camera.SetPosition(position_);
  main_camera.SetDirection(direction);
  main_camera.SetOrientation(up);
}
}  // namespace game_object
}  // namespace comet
