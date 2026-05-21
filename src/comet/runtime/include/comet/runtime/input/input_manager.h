// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_INPUT_INPUT_MANAGER_H_
#define COMET_RUNTIME_INPUT_INPUT_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/math/vector.h"
#include "comet/data/input/input.h"
#include "comet/runtime/input/input_backend.h"
#include "comet/runtime/manager.h"

namespace comet {
namespace input {
class InputManager : public Manager {
 public:
  static InputManager& Get();

  InputManager() = default;
  InputManager(const InputManager&) = delete;
  InputManager(InputManager&&) = delete;
  InputManager& operator=(const InputManager&) = delete;
  InputManager& operator=(InputManager&&) = delete;
  ~InputManager() override = default;

  void AttachBackend(InputBackend* backend);
  void DetachBackend();

  void Update();

  bool IsKeyPressed(KeyCode key) const;
  bool IsKeyDown(KeyCode key) const;
  bool IsKeyUp(KeyCode key) const;

  bool IsMousePressed(MouseButton button) const;
  bool IsMouseDown(MouseButton button) const;
  bool IsMouseUp(MouseButton button) const;

  bool IsAltPressed() const;
  bool IsShiftPressed() const;

  math::Vec2 GetMousePosition() const;
  math::Vec2 GetMouseDelta() const;
  math::Vec2 GetScrollDelta() const;

  void SetMousePosition(f32 x, f32 y);
  void EnableUnconstrainedMouseCursor();
  void DisableUnconstrainedMouseCursor();

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  static usize GetKeyIndex(KeyCode key);
  static usize GetMouseButtonIndex(MouseButton button);

  InputBackend* backend_{nullptr};

  InputSnapshot previous_snapshot_{};
  InputSnapshot current_snapshot_{};
  InputUserUpdate pending_update_{};
};
}  // namespace input
}  // namespace comet

#endif  // COMET_RUNTIME_INPUT_INPUT_MANAGER_H_