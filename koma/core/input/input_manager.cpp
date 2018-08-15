// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allow debugging memory leaks.
#include "../../debug.hpp"

#include "input_manager.hpp"

#include "../locator/locator.hpp"

namespace koma {
  bool InputManager::GetKey(KeyCode key_code) {
    return glfwGetKey(
      this->GetWindow(),
      static_cast<std::underlying_type_t<KeyCode>>(key_code)
    ) == GLFW_REPEAT;
  }

  bool InputManager::GetKeyUp(KeyCode key_code) {
    return glfwGetKey(
      this->GetWindow(),
      static_cast<std::underlying_type_t<KeyCode>>(key_code)
    ) == GLFW_RELEASE;
  }

  bool InputManager::GetKeyDown(KeyCode key_code) {
    return glfwGetKey(
      this->GetWindow(),
      static_cast<std::underlying_type_t<KeyCode>>(key_code)
    ) == GLFW_PRESS;
  }

  GLFWwindow *InputManager::GetWindow() {
    return const_cast<GLFWwindow *>(Locator::render_manager().window());
  }

  glm::vec2 InputManager::GetMousePosition() {
    double current_mouse_y_pos, current_mouse_x_pos;

    glfwGetCursorPos(
      this->GetWindow(),
      &current_mouse_x_pos,
      &current_mouse_y_pos
    );

    return glm::vec2((float)current_mouse_x_pos, (float)current_mouse_y_pos);
  }

  void InputManager::SetMousePosition(float x, float y) {
    glfwSetCursorPos(this->GetWindow(), x, y);
  }
};  // namespace koma
