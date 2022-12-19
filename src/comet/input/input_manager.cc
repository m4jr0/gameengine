// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "input_manager.h"

#include "comet/core/engine.h"

namespace comet {
namespace input {
void InputManager::Shutdown() {
  window_handle_ = nullptr;
  Manager::Shutdown();
}

void InputManager::Update() { glfwPollEvents(); }

bool InputManager::IsKeyPressed(KeyCode key_code) const {
  return glfwGetKey(window_handle_,
                    static_cast<std::underlying_type_t<KeyCode>>(key_code)) ==
         GLFW_REPEAT;
}

bool InputManager::IsKeyUp(KeyCode key_code) const {
  return glfwGetKey(window_handle_,
                    static_cast<std::underlying_type_t<KeyCode>>(key_code)) ==
         GLFW_RELEASE;
}

bool InputManager::IsKeyDown(KeyCode key_code) const {
  return glfwGetKey(window_handle_,
                    static_cast<std::underlying_type_t<KeyCode>>(key_code)) ==
         GLFW_PRESS;
}

bool InputManager::IsMousePressed(MouseCode key_code) const {
  return glfwGetMouseButton(window_handle_,
                            static_cast<std::underlying_type_t<MouseCode>>(
                                key_code)) == GLFW_REPEAT;
}

bool InputManager::IsMouseDown(MouseCode key_code) const {
  return glfwGetMouseButton(window_handle_,
                            static_cast<std::underlying_type_t<MouseCode>>(
                                key_code)) == GLFW_PRESS;
}

bool InputManager::IsMouseUp(MouseCode key_code) const {
  return glfwGetMouseButton(window_handle_,
                            static_cast<std::underlying_type_t<MouseCode>>(
                                key_code)) == GLFW_RELEASE;
}

void InputManager::AttachGlfwWindow(GLFWwindow* window_handle) {
  window_handle_ = window_handle;
}

glm::vec2 InputManager::GetMousePosition() const {
  f64 current_mouse_y_pos{0}, current_mouse_x_pos{0};

  glfwGetCursorPos(window_handle_, &current_mouse_x_pos, &current_mouse_y_pos);

  return glm::vec2(current_mouse_x_pos, current_mouse_y_pos);
}

void InputManager::SetMousePosition(f32 x, f32 y) {
  glfwSetCursorPos(window_handle_, x, y);
}
}  // namespace input
}  // namespace comet
