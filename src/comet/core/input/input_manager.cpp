// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "input_manager.hpp"

#include "core/engine.hpp"
#include "utils/logger.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

namespace comet {
void InputManager::Initialize() {
  glfwSetInputMode(cached_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputManager::Update() { glfwPollEvents(); }

bool InputManager::IsKeyBeingPressed(KeyCode key_code) const {
  return glfwGetKey(cached_window(),
                    static_cast<std::underlying_type_t<KeyCode>>(key_code)) ==
         GLFW_REPEAT;
}

bool InputManager::IsKeyUp(KeyCode key_code) const {
  return glfwGetKey(cached_window(),
                    static_cast<std::underlying_type_t<KeyCode>>(key_code)) ==
         GLFW_RELEASE;
}

bool InputManager::IsKeyDown(KeyCode key_code) const {
  return glfwGetKey(cached_window(),
                    static_cast<std::underlying_type_t<KeyCode>>(key_code)) ==
         GLFW_PRESS;
}

GLFWwindow *InputManager::cached_window() const {
  if (cached_window_ == nullptr) {
    cached_window_ =
        const_cast<GLFWwindow *>(Engine::engine()->render_manager()->window());
  }

  return cached_window_;
}

glm::vec2 InputManager::GetMousePosition() const {
  double current_mouse_y_pos = 0, current_mouse_x_pos = 0;

  glfwGetCursorPos(cached_window(), &current_mouse_x_pos, &current_mouse_y_pos);

  return glm::vec2(static_cast<float>(current_mouse_x_pos),
                   static_cast<float>(current_mouse_y_pos));
}

void InputManager::SetMousePosition(float x, float y) {
  glfwSetCursorPos(cached_window(), x, y);
}

}  // namespace comet
