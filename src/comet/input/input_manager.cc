// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "input_manager.h"

#include "comet/event/event_manager.h"
#include "comet/event/input_event.h"

#ifdef COMET_IMGUI
#include "imgui_impl_glfw.h"
#endif  // COMET_IMGUI

namespace comet {
namespace input {
InputManager& InputManager::Get() {
  static InputManager singleton{};
  return singleton;
}

void InputManager::Initialize() {
  Manager::Initialize();

  glfwSetScrollCallback(
      window_handle_, [](GLFWwindow* handle, f64 x_offset, f64 y_offset) {
#ifdef COMET_IMGUI
        if (is_imgui_) {
          ImGui_ImplGlfw_ScrollCallback(handle, x_offset, y_offset);
        }
#endif  // COMET_IMGUI
        event::EventManager::Get().FireEvent<event::MouseScrollEvent>(x_offset,
                                                                      y_offset);
      });

  glfwSetCursorPosCallback(
      window_handle_, [](GLFWwindow* handle, f64 x_pos, f64 y_pos) {
#ifdef COMET_IMGUI
        if (is_imgui_) {
          ImGui_ImplGlfw_CursorPosCallback(handle, x_pos, y_pos);
        }
#endif  // COMET_IMGUI
        event::EventManager::Get().FireEvent<event::MouseMoveEvent>(
            math::Vec2{x_pos, y_pos});
      });

  glfwSetKeyCallback(window_handle_, [](GLFWwindow* handle, s32 key,
                                        s32 scan_code, s32 action, s32 mods) {
#ifdef COMET_IMGUI
    if (is_imgui_) {
      ImGui_ImplGlfw_KeyCallback(handle, key, scan_code, action, mods);
    }
#endif  // COMET_IMGUI
    event::EventManager::Get().FireEvent<event::KeyboardEvent>(
        static_cast<input::KeyCode>(key),
        static_cast<input::ScanCode>(scan_code),
        static_cast<input::Action>(action), static_cast<input::Mods>(mods));
  });

  glfwSetMouseButtonCallback(window_handle_, [](GLFWwindow* handle,
                                                s32 raw_button, s32 raw_action,
                                                s32 raw_mods) {
#ifdef COMET_IMGUI
    if (is_imgui_) {
      ImGui_ImplGlfw_MouseButtonCallback(handle, raw_button, raw_action,
                                         raw_mods);
    }
#endif  // COMET_IMGUI

    auto action{static_cast<Action>(raw_action)};

    if (action == Action::Press) {
      event::EventManager::Get().FireEvent<event::MouseClickEvent>(
          static_cast<MouseButton>(raw_button), static_cast<Mods>(raw_mods));
    } else if (action == Action::Release) {
      event::EventManager::Get().FireEvent<event::MouseReleaseEvent>(
          static_cast<MouseButton>(raw_button), static_cast<Mods>(raw_mods));
    }
  });

  glfwSetWindowFocusCallback(
      window_handle_, [](GLFWwindow* handle, s32 is_focused) {
#ifdef COMET_IMGUI
        if (is_imgui_) {
          ImGui_ImplGlfw_WindowFocusCallback(handle, is_focused);
        }
#endif  // COMET_IMGUI
      });

  glfwSetCursorEnterCallback(
      window_handle_, [](GLFWwindow* handle, s32 is_entered) {
#ifdef COMET_IMGUI
        if (is_imgui_) {
          ImGui_ImplGlfw_CursorEnterCallback(handle, is_entered);
        }
#endif  // COMET_IMGUI
      });

  glfwSetCharCallback(window_handle_, [](GLFWwindow* handle, u32 code_point) {
#ifdef COMET_IMGUI
    if (is_imgui_) {
      ImGui_ImplGlfw_CharCallback(handle, code_point);
    }
#endif  // COMET_IMGUI
  });

  glfwSetMonitorCallback([](GLFWmonitor* monitor, s32 event) {
#ifdef COMET_IMGUI
    if (is_imgui_) {
      ImGui_ImplGlfw_MonitorCallback(monitor, event);
    }
#endif  // COMET_IMGUI
  });
}

void InputManager::Shutdown() {
  glfwSetScrollCallback(window_handle_, nullptr);
  glfwSetCursorPosCallback(window_handle_, nullptr);
  glfwSetKeyCallback(window_handle_, nullptr);
  glfwSetMouseButtonCallback(window_handle_, nullptr);
  glfwSetWindowFocusCallback(window_handle_, nullptr);
  glfwSetCursorEnterCallback(window_handle_, nullptr);
  glfwSetCharCallback(window_handle_, nullptr);
  glfwSetMonitorCallback(nullptr);
  window_handle_ = nullptr;
  Manager::Shutdown();
}

void InputManager::Update() { glfwPollEvents(); }

bool InputManager::IsKeyPressed(KeyCode key_code) const {
  return glfwGetKey(window_handle_,
                    static_cast<std::underlying_type_t<KeyCode>>(key_code)) ==
         GLFW_PRESS;
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

bool InputManager::IsMousePressed(MouseButton key_code) const {
  return glfwGetMouseButton(window_handle_,
                            static_cast<std::underlying_type_t<MouseButton>>(
                                key_code)) == GLFW_REPEAT;
}

bool InputManager::IsMouseDown(MouseButton key_code) const {
  return glfwGetMouseButton(window_handle_,
                            static_cast<std::underlying_type_t<MouseButton>>(
                                key_code)) == GLFW_PRESS;
}

bool InputManager::IsMouseUp(MouseButton key_code) const {
  return glfwGetMouseButton(window_handle_,
                            static_cast<std::underlying_type_t<MouseButton>>(
                                key_code)) == GLFW_RELEASE;
}

math::Vec2 InputManager::GetMousePosition() const {
  f64 current_mouse_y_pos;
  f64 current_mouse_x_pos;
  glfwGetCursorPos(window_handle_, &current_mouse_x_pos, &current_mouse_y_pos);
  return math::Vec2{current_mouse_x_pos, current_mouse_y_pos};
}

void InputManager::SetMousePosition(f32 x, f32 y) {
  glfwSetCursorPos(window_handle_, x, y);
}

void InputManager::EnableUnconstrainedMouseCursor() {
  glfwSetInputMode(window_handle_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputManager::DisableUnconstrainedMouseCursor() {
  glfwSetInputMode(window_handle_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void InputManager::AttachGlfwWindow(GLFWwindow* window_handle) {
  window_handle_ = window_handle;
}

void InputManager::EnableImGui() { is_imgui_ = true; }

bool InputManager::IsAltPressed() const {
  return IsKeyPressed(KeyCode::LeftAlt) || IsKeyPressed(KeyCode::RightAlt);
}

bool InputManager::IsShiftPressed() const {
  return IsKeyPressed(KeyCode::LeftShift) || IsKeyPressed(KeyCode::RightShift);
}
}  // namespace input
}  // namespace comet
