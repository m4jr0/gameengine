// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera_handler.h"

#include "comet/event/input_event.h"
#include "comet/math/vector.h"
#include "comet/rendering/camera/camera.h"

namespace comet {
namespace editor {
CameraHandler::CameraHandler(const CameraHandlerDescr& descr)
    : camera_manager_{descr.camera_manager},
      event_manager_{descr.event_manager},
      input_manager_{descr.input_manager},
      time_manager_{descr.time_manager} {
  COMET_ASSERT(camera_manager_ != nullptr, "Camera manager is null!");
  COMET_ASSERT(event_manager_ != nullptr, "Event manager is null!");
  COMET_ASSERT(input_manager_ != nullptr, "Input manager is null!");
  COMET_ASSERT(time_manager_ != nullptr, "Time manager is null!");
}

CameraHandler ::~CameraHandler() {
  COMET_ASSERT(
      !is_initialized_,
      "Destructor called for camera handler, but it is still initialized!");
}

void CameraHandler ::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize camera handler, but it is already done!");

  event_manager_->Register(COMET_EVENT_BIND_FUNCTION(CameraHandler::OnEvent),
                           event::KeyboardEvent::kStaticType_);

  event_manager_->Register(COMET_EVENT_BIND_FUNCTION(CameraHandler::OnEvent),
                           event::MouseMoveEvent::kStaticType_);

  event_manager_->Register(COMET_EVENT_BIND_FUNCTION(CameraHandler::OnEvent),
                           event::MouseScrollEvent::kStaticType_);

  event_manager_->Register(COMET_EVENT_BIND_FUNCTION(CameraHandler::OnEvent),
                           event::MouseClickEvent::kStaticType_);

  event_manager_->Register(COMET_EVENT_BIND_FUNCTION(CameraHandler::OnEvent),
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
  auto is_left{input_manager_->IsKeyPressed(input::KeyCode::Left)};
  auto is_right{input_manager_->IsKeyPressed(input::KeyCode::Right)};
  auto is_up{input_manager_->IsKeyPressed(input::KeyCode::Up)};
  auto is_down{input_manager_->IsKeyPressed(input::KeyCode::Down)};
  auto is_keyboard_key{is_left || is_right || is_up || is_down};

  if (!is_mouse_button && !is_keyboard_key) {
    return;
  }

  auto* camera{camera_manager_->GetMainCamera()};
  auto width{camera->GetWidth()};
  auto height{camera->GetHeight()};

  if (width == 0 || height == 0) {
    return;
  }

  auto delta_time{time_manager_->GetFixedDeltaTime()};

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

  auto is_out_of_bounds{false};

  if (current_mouse_pos_.x > width) {
    current_mouse_pos_.x = static_cast<s32>(current_mouse_pos_.x) % width;
    is_out_of_bounds = true;
  } else if (current_mouse_pos_.x < 0) {
    current_mouse_pos_.x =
        width - static_cast<s32>(current_mouse_pos_.x) % width;
    is_out_of_bounds = true;
  }

  if (current_mouse_pos_.y > height) {
    current_mouse_pos_.y = static_cast<s32>(current_mouse_pos_.y) % height;
    is_out_of_bounds = true;
  } else if (current_mouse_pos_.y < 0) {
    current_mouse_pos_.y =
        height - static_cast<s32>(current_mouse_pos_.y) % height;
    is_out_of_bounds = true;
  }

  if (is_out_of_bounds) {
    last_mouse_pos_ = current_mouse_pos_;
    input_manager_->SetMousePosition(current_mouse_pos_.x,
                                     current_mouse_pos_.y);
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

  if (event_type == event::KeyboardEvent::kStaticType_) {
    const auto& keyboard_event{static_cast<const event::KeyboardEvent&>(event)};
    auto is_press{keyboard_event.GetAction() == input::Action::Press};

    switch (keyboard_event.GetKey()) {
      case input::KeyCode::F:
        if (is_press) {
          // TODO(m4jr0): Focus on current selected mesh.
          camera_manager_->GetMainCamera()->Reset();
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

    switch (button) {
      case input::MouseButton::Left:
        if (input_manager_->IsAltPressed()) {
          ResetMousePosition();
          is_orbiting_from_mouse_ = true;
        }

        break;

      case input::MouseButton::Right:
        ResetMousePosition();

        if (input_manager_->IsAltPressed()) {
          is_zooming_from_mouse_ = true;
        } else {
          is_rotating_from_mouse_ = true;
        }

        break;

      case input::MouseButton::Middle:
        ResetMousePosition();
        is_panning_from_mouse_ = true;
        break;
    }

    return;

  } else if (event_type == event::MouseScrollEvent::kStaticType_) {
    const auto& mouse_scroll_event{
        static_cast<const event::MouseScrollEvent&>(event)};
    auto camera{camera_manager_->GetMainCamera()};
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
    }

    return;
  }
}
void CameraHandler::ResetMousePosition() {
  last_mouse_pos_ = current_mouse_pos_ = input_manager_->GetMousePosition();
}
}  // namespace editor
}  // namespace comet