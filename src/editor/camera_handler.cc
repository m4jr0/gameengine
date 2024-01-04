// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera_handler.h"

#include "comet/event/event_manager.h"
#include "comet/event/input_event.h"
#include "comet/input/input_manager.h"
#include "comet/math/vector.h"
#include "comet/rendering/camera/camera.h"
#include "comet/rendering/camera/camera_manager.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace editor {
CameraHandler ::~CameraHandler() {
  COMET_ASSERT(
      !is_initialized_,
      "Destructor called for camera handler, but it is still initialized!");
}

void CameraHandler ::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize camera handler, but it is already done!");

  auto& event_manager{event::EventManager::Get()};

  event_manager.Register(COMET_EVENT_BIND_FUNCTION(CameraHandler::OnEvent),
                         event::KeyboardEvent::kStaticType_);

  event_manager.Register(COMET_EVENT_BIND_FUNCTION(CameraHandler::OnEvent),
                         event::MouseMoveEvent::kStaticType_);

  event_manager.Register(COMET_EVENT_BIND_FUNCTION(CameraHandler::OnEvent),
                         event::MouseScrollEvent::kStaticType_);

  event_manager.Register(COMET_EVENT_BIND_FUNCTION(CameraHandler::OnEvent),
                         event::MouseClickEvent::kStaticType_);

  event_manager.Register(COMET_EVENT_BIND_FUNCTION(CameraHandler::OnEvent),
                         event::MouseReleaseEvent::kStaticType_);

  is_initialized_ = true;
}

void CameraHandler ::Shutdown() {
  COMET_ASSERT(is_initialized_,
               "Tried to shutdown camera handler, but it is not initialized!");
  is_initialized_ = false;
}

void CameraHandler::Update() {
  auto is_mouse_button{is_orbiting_from_mouse_ || is_rotating_from_mouse_ ||
                       is_panning_from_mouse_ || is_zooming_from_mouse_};
  auto& input_manager{input::InputManager::Get()};

  auto is_left{input_manager.IsKeyPressed(input::KeyCode::Left)};
  auto is_right{input_manager.IsKeyPressed(input::KeyCode::Right)};
  auto is_up{input_manager.IsKeyPressed(input::KeyCode::Up)};
  auto is_down{input_manager.IsKeyPressed(input::KeyCode::Down)};
  auto is_keyboard_key{is_left || is_right || is_up || is_down};

  if (!is_mouse_button && !is_keyboard_key) {
    return;
  }

  auto* camera{rendering::CameraManager::Get().GetMainCamera()};
  auto width{camera->GetWidth()};
  auto height{camera->GetHeight()};

  if (width == 0 || height == 0) {
    return;
  }

  if (is_keyboard_key) {
    math::Vec3 delta{0.0f};

    if (is_left) {
      delta.x += -1.0f;
    }

    if (is_right) {
      delta.x += 1.0f;
    }

    if (is_up) {
      delta.z += 1.0f;
    }

    if (is_down) {
      delta.z += -1.0f;
    }

    delta *= kKeyboardMovementSensitivity_;
    camera->Move(delta);
  }

  if (!is_mouse_button) {
    return;
  }

  auto mouse_pos_delta{last_mouse_pos_ - current_mouse_pos_};
  last_mouse_pos_ = current_mouse_pos_;
  auto is_mouse_moving{math::GetSquaredMagnitude(mouse_pos_delta) > 0.05f};

  if (!is_mouse_moving) {
    return;
  }

  if (is_orbiting_from_mouse_) {
    math::Vec3 delta{mouse_pos_delta.x, -mouse_pos_delta.y, 0.0f};
    delta *= kMouseOrbitSensitivity_;
    camera->Orbit(delta);
    return;
  }

  if (is_rotating_from_mouse_) {
    mouse_pos_delta *= kMouseRotationSensitivity_;
    camera->Rotate(mouse_pos_delta);
    return;
  }

  if (is_panning_from_mouse_) {
    math::Vec3 delta{-mouse_pos_delta.x, -mouse_pos_delta.y, 0.0f};
    delta *= kMousePanSensitivity_;
    camera->Move(delta);
    return;
  }

  if (is_zooming_from_mouse_) {
    math::Vec3 delta{0.0f, 0.0f, -mouse_pos_delta.x + mouse_pos_delta.y};
    delta *= kMouseZoomSensitivity_;
    camera->Move(delta);
    return;
  }
}

bool CameraHandler::IsInitialized() const noexcept { return is_initialized_; }

void CameraHandler::OnEvent(const event::Event& event) {
  const auto event_type{event.GetType()};
  auto& camera_manager{rendering::CameraManager::Get()};

  if (event_type == event::KeyboardEvent::kStaticType_) {
    const auto& keyboard_event{static_cast<const event::KeyboardEvent&>(event)};
    auto is_press{keyboard_event.GetAction() == input::Action::Press};

    switch (keyboard_event.GetKey()) {
      case input::KeyCode::F:
        if (is_press) {
          // TODO(m4jr0): Focus on current selected mesh.
          camera_manager.GetMainCamera()->Reset();
        }

        break;
    }

    return;

  } else if (event_type == event::MouseMoveEvent::kStaticType_) {
    const auto& mouse_move_event{
        static_cast<const event::MouseMoveEvent&>(event)};
    current_mouse_pos_ = mouse_move_event.GetPosition();
    return;

  } else if (event_type == event::MouseClickEvent::kStaticType_) {
    const auto& mouse_click_event{
        static_cast<const event::MouseClickEvent&>(event)};
    auto button{mouse_click_event.GetButton()};
    auto& input_manager{input::InputManager::Get()};

    switch (button) {
      case input::MouseButton::Left:
        if (input_manager.IsAltPressed()) {
          is_orbiting_from_mouse_ = true;
        }

        break;

      case input::MouseButton::Right:
        if (input_manager.IsAltPressed()) {
          is_zooming_from_mouse_ = true;
        } else {
          is_rotating_from_mouse_ = true;
        }

        break;

      case input::MouseButton::Middle:
        is_panning_from_mouse_ = true;
        break;

      default:
        return;
    }

    ResetMousePosition();
    input_manager.EnableUnconstrainedMouseCursor();

    return;

  } else if (event_type == event::MouseScrollEvent::kStaticType_) {
    const auto& mouse_scroll_event{
        static_cast<const event::MouseScrollEvent&>(event)};
    auto camera{camera_manager.GetMainCamera()};
    camera->Move(math::Vec3(0.0f, 0.0f,
                            static_cast<f32>(mouse_scroll_event.GetYOffset())));
    return;

  } else if (event_type == event::MouseReleaseEvent::kStaticType_) {
    const auto& mouse_release_event{
        static_cast<const event::MouseReleaseEvent&>(event)};
    auto button{mouse_release_event.GetButton()};

    switch (button) {
      case input::MouseButton::Left:
        is_orbiting_from_mouse_ = false;
        break;

      case input::MouseButton::Right:
        is_rotating_from_mouse_ = false;
        is_zooming_from_mouse_ = false;
        break;

      case input::MouseButton::Middle:
        is_panning_from_mouse_ = false;
        break;

      default:
        return;
    }

    input::InputManager::Get().DisableUnconstrainedMouseCursor();
    return;
  }
}
void CameraHandler::ResetMousePosition() {
  last_mouse_pos_ = current_mouse_pos_ =
      input::InputManager::Get().GetMousePosition();
}
}  // namespace editor
}  // namespace comet