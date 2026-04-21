// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "camera_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/input/input_event.h"
#include "comet/input/input_manager.h"
#include "comet/math/vector.h"
#include "comet/rendering/camera_manager.h"

namespace comet {
namespace editor {
CameraHandler::~CameraHandler() {
  COMET_ASSERT(!is_initialized_, "CameraHandler::~CameraHandler",
               "camera handler is still initialized");
}

void CameraHandler::Initialize() {
  COMET_ASSERT(!is_initialized_, "CameraHandler::Initialize",
               "camera handler is already initialized");

  RegisterEvents();

  camera_ = rendering::CameraManager::Get().GetMainCamera();
  COMET_ASSERT(camera_, "CameraHandler::Initialize",
               "main camera handle is invalid");

  is_initialized_ = true;
}

void CameraHandler::Shutdown() {
  COMET_ASSERT(is_initialized_, "CameraHandler::Shutdown",
               "camera handler is not initialized");

  UnregisterEvents();

  camera_.Invalidate();
  is_initialized_ = false;
}

void CameraHandler::Update() {
  COMET_ASSERT(is_initialized_, "CameraHandler::Update",
               "camera handler is not initialized");

  const auto is_mouse_button{is_orbiting_from_mouse_ ||
                             is_rotating_from_mouse_ ||
                             is_panning_from_mouse_ || is_zooming_from_mouse_};
  auto& input_manager{input::InputManager::Get()};

  const auto is_left{input_manager.IsKeyPressed(input::KeyCode::Left)};
  const auto is_right{input_manager.IsKeyPressed(input::KeyCode::Right)};
  const auto is_up{input_manager.IsKeyPressed(input::KeyCode::Up)};
  const auto is_down{input_manager.IsKeyPressed(input::KeyCode::Down)};
  const auto is_keyboard_key{is_left || is_right || is_up || is_down};

  if (!is_mouse_button && !is_keyboard_key) {
    return;
  }

  auto& camera_manager{rendering::CameraManager::Get()};
  COMET_ASSERT(camera_manager.IsAlive(camera_), "CameraHandler::Update",
               "camera is invalid");

  const auto width{camera_manager.GetWidth(camera_)};
  const auto height{camera_manager.GetHeight(camera_)};

  if (width == 0 || height == 0) {
    return;
  }

  if (is_keyboard_key) {
    math::Vec3 delta{.0f};

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
    camera_manager.Move(camera_, delta);
  }

  if (!is_mouse_button) {
    return;
  }

  auto mouse_pos_delta{last_mouse_pos_ - current_mouse_pos_};
  last_mouse_pos_ = current_mouse_pos_;
  const auto is_mouse_moving{math::GetSquaredMagnitude(mouse_pos_delta) > .05f};

  if (!is_mouse_moving) {
    return;
  }

  if (is_orbiting_from_mouse_) {
    math::Vec2 delta{mouse_pos_delta.x, -mouse_pos_delta.y};
    delta *= kMouseOrbitSensitivity_;
    camera_manager.Orbit(camera_, delta);
    return;
  }

  if (is_rotating_from_mouse_) {
    mouse_pos_delta *= kMouseRotationSensitivity_;
    camera_manager.Rotate(camera_, mouse_pos_delta);
    return;
  }

  if (is_panning_from_mouse_) {
    math::Vec3 delta{-mouse_pos_delta.x, -mouse_pos_delta.y, .0f};
    delta *= kMousePanSensitivity_;
    camera_manager.Move(camera_, delta);
    return;
  }

  if (is_zooming_from_mouse_) {
    math::Vec3 delta{.0f, .0f, -mouse_pos_delta.x + mouse_pos_delta.y};
    delta *= kMouseZoomSensitivity_;
    camera_manager.Move(camera_, delta);
    return;
  }
}

bool CameraHandler::IsInitialized() const noexcept { return is_initialized_; }

void CameraHandler::OnEvent(const event::Event& event) {
  COMET_ASSERT(is_initialized_, "CameraHandler::OnEvent",
               "camera handler is not initialized");

  const auto event_type{event.GetType()};
  auto& camera_manager{rendering::CameraManager::Get()};

  if (!camera_manager.IsAlive(camera_)) {
    return;
  }

  if (event_type == input::KeyboardEvent::kStaticType_) {
    const auto& keyboard_event{static_cast<const input::KeyboardEvent&>(event)};
    const auto is_press{keyboard_event.GetAction() == input::Action::Press};

    switch (keyboard_event.GetKey()) {
      case input::KeyCode::F:
        if (is_press) {
          camera_manager.Reset(camera_);
        }
        break;

      default:
        return;
    }

    return;
  }

  if (event_type == input::MouseMoveEvent::kStaticType_) {
    const auto& mouse_move_event{
        static_cast<const input::MouseMoveEvent&>(event)};
    current_mouse_pos_ = mouse_move_event.GetPosition();
    return;
  }

  if (event_type == input::MouseClickEvent::kStaticType_) {
    const auto& mouse_click_event{
        static_cast<const input::MouseClickEvent&>(event)};
    const auto button{mouse_click_event.GetButton()};
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
  }

  if (event_type == input::MouseScrollEvent::kStaticType_) {
    const auto& mouse_scroll_event{
        static_cast<const input::MouseScrollEvent&>(event)};
    camera_manager.Move(
        camera_, math::Vec3(.0f, .0f,
                            static_cast<f32>(mouse_scroll_event.GetYOffset())));
    return;
  }

  if (event_type == input::MouseReleaseEvent::kStaticType_) {
    const auto& mouse_release_event{
        static_cast<const input::MouseReleaseEvent&>(event)};
    const auto button{mouse_release_event.GetButton()};

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
  }
}

void CameraHandler::RegisterEvents() {
  auto& event_manager{event::EventManager::Get()};
  const auto event_function{
      [this](const event::Event& event) { OnEvent(event); }};

  keyboard_listener_id_ = event_manager.Register(
      event_function, input::KeyboardEvent::kStaticType_);
  COMET_ASSERT(keyboard_listener_id_ != event::kInvalidEventListenerId,
               "CameraHandler::RegisterEvents",
               "keyboard listener registration failed");

  mouse_move_listener_id_ = event_manager.Register(
      event_function, input::MouseMoveEvent::kStaticType_);
  COMET_ASSERT(mouse_move_listener_id_ != event::kInvalidEventListenerId,
               "CameraHandler::RegisterEvents",
               "mouse move listener registration failed");

  mouse_scroll_listener_id_ = event_manager.Register(
      event_function, input::MouseScrollEvent::kStaticType_);
  COMET_ASSERT(mouse_scroll_listener_id_ != event::kInvalidEventListenerId,
               "CameraHandler::RegisterEvents",
               "mouse scroll listener registration failed");

  mouse_click_listener_id_ = event_manager.Register(
      event_function, input::MouseClickEvent::kStaticType_);
  COMET_ASSERT(mouse_click_listener_id_ != event::kInvalidEventListenerId,
               "CameraHandler::RegisterEvents",
               "mouse click listener registration failed");

  mouse_release_listener_id_ = event_manager.Register(
      event_function, input::MouseReleaseEvent::kStaticType_);
  COMET_ASSERT(mouse_release_listener_id_ != event::kInvalidEventListenerId,
               "CameraHandler::RegisterEvents",
               "mouse release listener registration failed");
}

void CameraHandler::UnregisterEvents() {
  auto& event_manager{event::EventManager::Get()};

  if (keyboard_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(keyboard_listener_id_);
    keyboard_listener_id_ = event::kInvalidEventListenerId;
  }

  if (mouse_move_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(mouse_move_listener_id_);
    mouse_move_listener_id_ = event::kInvalidEventListenerId;
  }

  if (mouse_scroll_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(mouse_scroll_listener_id_);
    mouse_scroll_listener_id_ = event::kInvalidEventListenerId;
  }

  if (mouse_click_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(mouse_click_listener_id_);
    mouse_click_listener_id_ = event::kInvalidEventListenerId;
  }

  if (mouse_release_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(mouse_release_listener_id_);
    mouse_release_listener_id_ = event::kInvalidEventListenerId;
  }
}

void CameraHandler::ResetMousePosition() {
  COMET_ASSERT(is_initialized_, "CameraHandler::ResetMousePosition",
               "camera handler is not initialized");

  last_mouse_pos_ = current_mouse_pos_ =
      input::InputManager::Get().GetMousePosition();
}
}  // namespace editor
}  // namespace comet