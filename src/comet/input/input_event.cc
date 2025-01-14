// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "input_event.h"

namespace comet {
namespace input {
const stringid::StringId KeyboardEvent::kStaticType_{
    COMET_STRING_ID("event_keyboard")};

KeyboardEvent::KeyboardEvent(input::KeyCode key, input::ScanCode scan_code,
                             input::Action action, input::Mods mods)
    : key_{key}, scan_code_{scan_code}, action_{action}, mods_{mods} {}

stringid::StringId KeyboardEvent::GetType() const noexcept {
  return kStaticType_;
}

input::KeyCode KeyboardEvent::GetKey() const noexcept { return key_; }

input::ScanCode KeyboardEvent::GetScanCode() const noexcept {
  return scan_code_;
}

input::Action KeyboardEvent::GetAction() const noexcept { return action_; }

input::Mods KeyboardEvent::GetMods() const noexcept { return mods_; }

const stringid::StringId MouseMoveEvent::kStaticType_{
    COMET_STRING_ID("event_mouse_move")};

MouseMoveEvent::MouseMoveEvent(const math::Vec2& position)
    : position_{position} {}

stringid::StringId MouseMoveEvent::GetType() const noexcept {
  return kStaticType_;
}

const math::Vec2& MouseMoveEvent::GetPosition() const noexcept {
  return position_;
}

MouseScrollEvent::MouseScrollEvent(f64 x_offset, f64 y_offset)
    : x_offset_{x_offset}, y_offset_{y_offset} {}

stringid::StringId MouseScrollEvent::GetType() const noexcept {
  return kStaticType_;
}

const stringid::StringId MouseScrollEvent::kStaticType_{
    COMET_STRING_ID("event_mouse_scroll")};

f64 MouseScrollEvent::GetXOffset() const noexcept { return x_offset_; }

f64 MouseScrollEvent::GetYOffset() const noexcept { return y_offset_; }

const stringid::StringId MouseClickEvent::kStaticType_{
    COMET_STRING_ID("event_mouse_click")};

MouseClickEvent::MouseClickEvent(input::MouseButton button, input::Mods mods)
    : button_{button}, mods_{mods} {}

stringid::StringId MouseClickEvent::GetType() const noexcept {
  return kStaticType_;
}

input::MouseButton MouseClickEvent::GetButton() const noexcept {
  return button_;
}

input::Mods MouseClickEvent::GetMods() const noexcept { return mods_; }

const stringid::StringId MouseReleaseEvent::kStaticType_{
    COMET_STRING_ID("event_mouse_release")};

MouseReleaseEvent::MouseReleaseEvent(input::MouseButton button,
                                     input::Mods mods)
    : button_{button}, mods_{mods} {}

stringid::StringId MouseReleaseEvent::GetType() const noexcept {
  return kStaticType_;
}

input::MouseButton MouseReleaseEvent::GetButton() const noexcept {
  return button_;
}

input::Mods MouseReleaseEvent::GetMods() const noexcept { return mods_; }
}  // namespace input
}  // namespace comet
