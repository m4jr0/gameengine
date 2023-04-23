// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_INPUT_INPUT_MANAGER_H_
#define COMET_COMET_INPUT_INPUT_MANAGER_H_

#include "comet_precompile.h"

#include "glm/glm.hpp"

#include "comet/core/manager.h"
#include "comet/input/input.h"
#include "comet/rendering/window/glfw/glfw_window.h"

namespace comet {
namespace input {
class InputManager : public Manager {
 public:
  InputManager() = default;
  InputManager(const InputManager&) = delete;
  InputManager(InputManager&&) = delete;
  InputManager& operator=(const InputManager&) = delete;
  InputManager& operator=(InputManager&&) = delete;
  virtual ~InputManager() = default;

  void Initialize() override;
  virtual void Shutdown() override;
  virtual void Update();
  virtual bool IsKeyPressed(KeyCode) const;
  virtual bool IsKeyUp(KeyCode) const;
  virtual bool IsKeyDown(KeyCode) const;
  virtual bool IsMousePressed(MouseButton key_code) const;
  virtual bool IsMouseDown(MouseButton key_code) const;
  virtual bool IsMouseUp(MouseButton key_code) const;
  virtual glm::vec2 GetMousePosition() const;
  virtual void SetMousePosition(f32, f32);
  void AttachGlfwWindow(GLFWwindow* window_handle_);
  bool IsAltPressed() const;
  bool IsShiftPressed() const;

 private:
  mutable std::atomic<GLFWwindow*> window_handle_{nullptr};
};

class NullInputManager : public InputManager {
 public:
  NullInputManager() = default;
  NullInputManager(const NullInputManager&) = delete;
  NullInputManager(NullInputManager&&) = delete;
  NullInputManager& operator=(const NullInputManager&) = delete;
  NullInputManager& operator=(NullInputManager&&) = delete;
  virtual ~NullInputManager() = default;

  virtual bool IsKeyPressed(KeyCode key_code) const override { return false; };

  virtual bool IsKeyUp(KeyCode key_code) const override { return false; };
  virtual bool IsKeyDown(KeyCode key_code) const override { return false; };
  virtual bool IsMousePressed(MouseButton key_code) const override {
    return false;
  };
  virtual bool IsMouseDown(MouseButton key_code) const override {
    return false;
  };
  virtual bool IsMouseUp(MouseButton key_code) const override { return false; };

  virtual glm::vec2 GetMousePosition() const override {
    return glm::vec2{0.0f, 0.0f};
  };

  virtual void SetMousePosition(f32 x, f32 y) override{};
};
}  // namespace input
}  // namespace comet

#endif  // COMET_COMET_INPUT_INPUT_MANAGER_H_
