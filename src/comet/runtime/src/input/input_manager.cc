// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/input/input_manager.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace input {
InputManager& InputManager::Get() {
  static InputManager singleton{};
  return singleton;
}

void InputManager::AttachBackend(InputBackend* backend) {
  COMET_ASSERT(backend != nullptr, "InputManager::AttachBackend",
               "backend is null");
  backend_ = backend;
}

void InputManager::DetachBackend() { backend_ = nullptr; }

void InputManager::Update() {
  COMET_ASSERT(backend_ != nullptr, "InputManager::Update",
               "input backend is null");

  previous_snapshot_ = current_snapshot_;
  current_snapshot_.scroll_delta = {};
  backend_->Poll(current_snapshot_);
  backend_->ApplyUserUpdate(pending_update_);
  pending_update_ = {};
}

bool InputManager::IsKeyPressed(KeyCode key) const {
  const auto index{GetKeyIndex(key)};
  return current_snapshot_.keys[index] && !previous_snapshot_.keys[index];
}

bool InputManager::IsKeyDown(KeyCode key) const {
  return current_snapshot_.keys[GetKeyIndex(key)];
}

bool InputManager::IsKeyUp(KeyCode key) const {
  const auto index{GetKeyIndex(key)};
  return !current_snapshot_.keys[index] && previous_snapshot_.keys[index];
}

bool InputManager::IsMousePressed(MouseButton button) const {
  const auto index{GetMouseButtonIndex(button)};
  return current_snapshot_.mouse_buttons[index] &&
         !previous_snapshot_.mouse_buttons[index];
}

bool InputManager::IsMouseDown(MouseButton button) const {
  return current_snapshot_.mouse_buttons[GetMouseButtonIndex(button)];
}

bool InputManager::IsMouseUp(MouseButton button) const {
  const auto index{GetMouseButtonIndex(button)};
  return !current_snapshot_.mouse_buttons[index] &&
         previous_snapshot_.mouse_buttons[index];
}

bool InputManager::IsAltPressed() const {
  return IsKeyDown(KeyCode::LeftAlt) || IsKeyDown(KeyCode::RightAlt);
}

bool InputManager::IsShiftPressed() const {
  return IsKeyDown(KeyCode::LeftShift) || IsKeyDown(KeyCode::RightShift);
}

math::Vec2 InputManager::GetMousePosition() const {
  return current_snapshot_.mouse_position;
}

math::Vec2 InputManager::GetMouseDelta() const {
  return current_snapshot_.mouse_position - previous_snapshot_.mouse_position;
}

math::Vec2 InputManager::GetScrollDelta() const {
  return current_snapshot_.scroll_delta;
}

void InputManager::SetMousePosition(f32 x, f32 y) {
  pending_update_.has_mouse_position = true;
  pending_update_.mouse_position = {x, y};
}

void InputManager::EnableUnconstrainedMouseCursor() {
  pending_update_.cursor_mode = MouseCursorMode::Disabled;
}

void InputManager::DisableUnconstrainedMouseCursor() {
  pending_update_.cursor_mode = MouseCursorMode::Normal;
}

void InputManager::OnInitialize() {}

void InputManager::OnShutdown() {
  DetachBackend();
  previous_snapshot_ = {};
  current_snapshot_ = {};
  pending_update_ = {};
}

usize InputManager::GetKeyIndex(KeyCode key) {
  const auto value{static_cast<s32>(key)};
  COMET_ASSERT(value >= internal::kKeyBaseOffset, "InputManager::GetKeyIndex",
               "key is out of range", "key", value);
  return static_cast<usize>(value - internal::kKeyBaseOffset);
}

usize InputManager::GetMouseButtonIndex(MouseButton button) {
  const auto value{static_cast<s32>(button)};
  COMET_ASSERT(
      value >= 0 && static_cast<usize>(value) < internal::kMouseButtonCount,
      "InputManager::GetMouseButtonIndex", "mouse button is out of range",
      "button", value);
  return static_cast<usize>(value);
}
}  // namespace input
}  // namespace comet