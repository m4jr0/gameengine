// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "input_event.h"

namespace comet {
namespace event {
const core::StringId KeyboardEvent::kStaticType_ =
    COMET_STRING_ID("event_keyboard");

KeyboardEvent::KeyboardEvent(int key, int scan_code, int action, int mods)
    : key_(key), scan_code_(scan_code), action_(action), mods_(mods) {}

const core::StringId& KeyboardEvent::GetType() const noexcept {
  return kStaticType_;
}

int KeyboardEvent::GetKey() const noexcept { return key_; }

int KeyboardEvent::GetScanCode() const noexcept { return scan_code_; }

int KeyboardEvent::GetAction() const noexcept { return action_; }

int KeyboardEvent::GetMods() const noexcept { return mods_; }

const core::StringId MouseMoveEvent::kStaticType_ =
    COMET_STRING_ID("event_mouse_move");

MouseMoveEvent::MouseMoveEvent(glm::vec2 position) : position_(position) {}

const core::StringId& MouseMoveEvent::GetType() const noexcept {
  return kStaticType_;
}

glm::vec2 MouseMoveEvent::GetPosition() const noexcept { return position_; }

MouseScrollEvent::MouseScrollEvent(double x_offset, double y_offset)
    : x_offset_(x_offset), y_offset_(y_offset) {}

const core::StringId& MouseScrollEvent::GetType() const noexcept {
  return kStaticType_;
}

const core::StringId MouseScrollEvent::kStaticType_ =
    COMET_STRING_ID("event_mouse_scroll");

double MouseScrollEvent::GetXOffset() const noexcept { return x_offset_; }

double MouseScrollEvent::GetYOffset() const noexcept { return y_offset_; }

const core::StringId MouseClickEvent::kStaticType_ =
    COMET_STRING_ID("event_mouse_click");

MouseClickEvent::MouseClickEvent(bool is_left_button, bool is_right_button,
                                 bool is_middle_button)
    : is_left_button_(is_left_button),
      is_right_button_(is_right_button),
      is_middle_button_(is_middle_button) {}

const core::StringId& MouseClickEvent::GetType() const noexcept {
  return kStaticType_;
}
bool MouseClickEvent::IsLeftButton() const noexcept { return is_left_button_; }

bool MouseClickEvent::IsRightButton() const noexcept {
  return is_right_button_;
}

bool MouseClickEvent::IsMiddleButton() const noexcept {
  return is_middle_button_;
}

const core::StringId MouseReleaseEvent::kStaticType_ =
    COMET_STRING_ID("event_mouse_release");

MouseReleaseEvent::MouseReleaseEvent(bool is_left_button, bool is_right_button,
                                     bool is_middle_button)
    : is_left_button_(is_left_button),
      is_right_button_(is_right_button),
      is_middle_button_(is_middle_button) {}

const core::StringId& MouseReleaseEvent::GetType() const noexcept {
  return kStaticType_;
}

bool MouseReleaseEvent::IsLeftButton() const noexcept {
  return is_left_button_;
}

bool MouseReleaseEvent::IsRightButton() const noexcept {
  return is_right_button_;
}
bool MouseReleaseEvent::IsMiddleButton() const noexcept {
  return is_middle_button_;
}
}  // namespace event
}  // namespace comet
