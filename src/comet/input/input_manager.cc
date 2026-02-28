// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "input_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/thread/thread_context.h"
#include "comet/core/game_state_manager.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/event/event_manager.h"
#include "comet/input/input_event.h"

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
  thread_id_ = thread::GetThreadId();

  cached_input_state_.keys_pressed =
      Bitset(&cached_input_state_allocator_, internal::kKeyCount);
  cached_input_state_.keys_down =
      Bitset(&cached_input_state_allocator_, internal::kKeyCount);
  cached_input_state_.keys_up =
      Bitset(&cached_input_state_allocator_, internal::kKeyCount);

  cached_input_state_.mouse_buttons_pressed =
      Bitset(&cached_input_state_allocator_, internal::kMouseButtonCount);
  cached_input_state_.mouse_buttons_down =
      Bitset(&cached_input_state_allocator_, internal::kMouseButtonCount);
  cached_input_state_.mouse_buttons_up =
      Bitset(&cached_input_state_allocator_, internal::kMouseButtonCount);

  glfwSetScrollCallback(window_handle_, []([[maybe_unused]] GLFWwindow* handle,
                                           f64 x_offset, f64 y_offset) {
#ifdef COMET_IMGUI
    if (is_imgui_) {
      ImGui_ImplGlfw_ScrollCallback(handle, x_offset, y_offset);

      if (ImGui::GetIO().WantCaptureMouse) {
        return;
      }
    }
#endif  // COMET_IMGUI

    if (GameStateManager::Get().IsPaused()) {
      return;
    }

    event::EventManager::Get().FireEvent<MouseScrollEvent>(x_offset, y_offset);
  });

  glfwSetCursorPosCallback(
      window_handle_,
      []([[maybe_unused]] GLFWwindow* handle, f64 x_pos, f64 y_pos) {
#ifdef COMET_IMGUI
        if (is_imgui_) {
          ImGui_ImplGlfw_CursorPosCallback(handle, x_pos, y_pos);
        }
#endif  // COMET_IMGUI

        if (GameStateManager::Get().IsPaused()) {
          return;
        }

        event::EventManager::Get().FireEvent<MouseMoveEvent>(
            math::Vec2{x_pos, y_pos});
      });

  glfwSetKeyCallback(
      window_handle_, []([[maybe_unused]] GLFWwindow* handle, s32 key,
                         s32 scan_code, s32 action, s32 mods) {
#ifdef COMET_IMGUI
        if (is_imgui_) {
          ImGui_ImplGlfw_KeyCallback(handle, key, scan_code, action, mods);

          if (ImGui::GetIO().WantCaptureKeyboard) {
            return;
          }
        }
#endif  // COMET_IMGUI

        if (GameStateManager::Get().IsPaused()) {
          return;
        }

        event::EventManager::Get().FireEvent<KeyboardEvent>(
            static_cast<input::KeyCode>(key),
            static_cast<input::ScanCode>(scan_code),
            static_cast<input::Action>(action), static_cast<input::Mods>(mods));
      });

  glfwSetMouseButtonCallback(
      window_handle_, []([[maybe_unused]] GLFWwindow* handle, s32 raw_button,
                         s32 raw_action, s32 raw_mods) {
#ifdef COMET_IMGUI
        if (is_imgui_) {
          ImGui_ImplGlfw_MouseButtonCallback(handle, raw_button, raw_action,
                                             raw_mods);

          if (ImGui::GetIO().WantCaptureMouse) {
            return;
          }
        }
#endif  // COMET_IMGUI

        if (GameStateManager::Get().IsPaused()) {
          return;
        }

        auto action{static_cast<Action>(raw_action)};

        if (action == Action::Press) {
          event::EventManager::Get().FireEvent<MouseClickEvent>(
              static_cast<MouseButton>(raw_button),
              static_cast<Mods>(raw_mods));
        } else if (action == Action::Release) {
          event::EventManager::Get().FireEvent<MouseReleaseEvent>(
              static_cast<MouseButton>(raw_button),
              static_cast<Mods>(raw_mods));
        }
      });

  glfwSetWindowFocusCallback(
      window_handle_,
      []([[maybe_unused]] GLFWwindow* handle, [[maybe_unused]] s32 is_focused) {
#ifdef COMET_IMGUI
        if (is_imgui_) {
          ImGui_ImplGlfw_WindowFocusCallback(handle, is_focused);
        }
#endif  // COMET_IMGUI
      });

  glfwSetCursorEnterCallback(
      window_handle_,
      []([[maybe_unused]] GLFWwindow* handle, [[maybe_unused]] s32 is_entered) {
#ifdef COMET_IMGUI
        if (is_imgui_) {
          ImGui_ImplGlfw_CursorEnterCallback(handle, is_entered);
        }
#endif  // COMET_IMGUI
      });

  glfwSetCharCallback(window_handle_, []([[maybe_unused]] GLFWwindow* handle,
                                         [[maybe_unused]] u32 code_point) {
#ifdef COMET_IMGUI
    if (is_imgui_) {
      ImGui_ImplGlfw_CharCallback(handle, code_point);
    }
#endif  // COMET_IMGUI
  });

  glfwSetMonitorCallback(
      []([[maybe_unused]] GLFWmonitor* monitor, [[maybe_unused]] s32 event) {
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

  cached_input_state_.keys_pressed.Destroy();
  cached_input_state_.keys_down.Destroy();
  cached_input_state_.keys_up.Destroy();

  cached_input_state_.mouse_buttons_pressed.Destroy();
  cached_input_state_.mouse_buttons_down.Destroy();
  cached_input_state_.mouse_buttons_up.Destroy();

  thread_id_ = thread::kInvalidThreadId;
  Manager::Shutdown();
}

void InputManager::Update() {
  COMET_ASSERT(thread_id_ == thread::GetThreadId(),
               "Input Manager was initialized on thread #", thread_id_,
               ", but GLFW is not thread-safe. The Update method MUST be "
               "called from the "
               "same thread.");

  ReadInputs();
  ApplyUserUpdates();
}

bool InputManager::IsKeyPressed(KeyCode key_code) const {
  if (GameStateManager::Get().IsPaused()) {
    return false;
  }

  return cached_input_state_.keys_pressed.Test(static_cast<usize>(key_code) -
                                               internal::kKeyBaseOffset);
}

bool InputManager::IsKeyUp(KeyCode key_code) const {
  if (GameStateManager::Get().IsPaused()) {
    return false;
  }

  return cached_input_state_.keys_up.Test(static_cast<usize>(key_code) -
                                          internal::kKeyBaseOffset);
}

bool InputManager::IsKeyDown(KeyCode key_code) const {
  if (GameStateManager::Get().IsPaused()) {
    return false;
  }

  return cached_input_state_.keys_down.Test(static_cast<usize>(key_code) -
                                            internal::kKeyBaseOffset);
}

bool InputManager::IsMousePressed(MouseButton key_code) const {
  if (GameStateManager::Get().IsPaused()) {
    return false;
  }

  return cached_input_state_.mouse_buttons_pressed.Test(
      static_cast<usize>(key_code));
}

bool InputManager::IsMouseDown(MouseButton key_code) const {
  if (GameStateManager::Get().IsPaused()) {
    return false;
  }

  return cached_input_state_.mouse_buttons_down.Test(
      static_cast<usize>(key_code));
}

bool InputManager::IsMouseUp(MouseButton key_code) const {
  if (GameStateManager::Get().IsPaused()) {
    return false;
  }

  return cached_input_state_.mouse_buttons_up.Test(
      static_cast<usize>(key_code));
}

math::Vec2 InputManager::GetMousePosition() const {
  return cached_input_state_.mouse_position;
}

void InputManager::SetMousePosition(f32 x, f32 y) {
  fiber::FiberLockGuard lock{updated_input_state_.mtx};
  updated_input_state_.new_position = math::Vec2{x, y};
}

void InputManager::EnableUnconstrainedMouseCursor() {
  fiber::FiberLockGuard lock{updated_input_state_.mtx};
  updated_input_state_.cursor_mode = MouseCursorMode::Disabled;
}

void InputManager::DisableUnconstrainedMouseCursor() {
  fiber::FiberLockGuard lock{updated_input_state_.mtx};
  updated_input_state_.cursor_mode = MouseCursorMode::Normal;
}

void InputManager::AttachGlfwWindow(GLFWwindow* window_handle) {
  window_handle_ = window_handle;
}

#ifdef COMET_IMGUI
void InputManager::EnableImGui() { is_imgui_ = true; }
#endif  // COMET_IMGUI

bool InputManager::IsAltPressed() const {
  if (GameStateManager::Get().IsPaused()) {
    return false;
  }

  return IsKeyPressed(KeyCode::LeftAlt) || IsKeyPressed(KeyCode::RightAlt);
}

bool InputManager::IsShiftPressed() const {
  if (GameStateManager::Get().IsPaused()) {
    return false;
  }

  return IsKeyPressed(KeyCode::LeftShift) || IsKeyPressed(KeyCode::RightShift);
}

void InputManager::ReadInputs() {
  glfwPollEvents();

  for (usize i{0}; i < internal::kKeyCount; ++i) {
    auto glfw_key{glfwGetKey(window_handle_,
                             static_cast<s32>(i) + internal::kKeyBaseOffset)};

    if (glfw_key == GLFW_PRESS) {
      cached_input_state_.keys_pressed.Set(i);
      cached_input_state_.keys_down.Set(i);
    } else {
      cached_input_state_.keys_pressed.Reset(i);
      cached_input_state_.keys_down.Reset(i);
    }

    if (glfw_key == GLFW_RELEASE) {
      cached_input_state_.keys_up.Set(i);
    } else {
      cached_input_state_.keys_up.Reset(i);
    }
  }

  for (usize i{0}; i < internal::kMouseButtonCount; ++i) {
    auto glfw_mouse_button{
        glfwGetMouseButton(window_handle_, static_cast<s32>(i))};

    if (glfw_mouse_button == GLFW_PRESS) {
      cached_input_state_.mouse_buttons_pressed.Set(i);
      cached_input_state_.mouse_buttons_down.Set(i);
    } else {
      cached_input_state_.mouse_buttons_pressed.Reset(i);
      cached_input_state_.mouse_buttons_down.Reset(i);
    }

    if (glfw_mouse_button == GLFW_RELEASE) {
      cached_input_state_.mouse_buttons_up.Set(i);
    } else {
      cached_input_state_.mouse_buttons_up.Reset(i);
    }
  }

  f64 x;
  f64 y;
  glfwGetCursorPos(window_handle_, &x, &y);
  cached_input_state_.mouse_position.x = static_cast<f32>(x);
  cached_input_state_.mouse_position.y = static_cast<f32>(y);
}

void InputManager::ApplyUserUpdates() {
  if (updated_input_state_.new_position.has_value()) {
    const auto& new_position{updated_input_state_.new_position.value()};
    glfwSetCursorPos(window_handle_, new_position.x, new_position.y);
    updated_input_state_.new_position.reset();
  }

  switch (updated_input_state_.cursor_mode) {
    case MouseCursorMode::Normal:
      glfwSetInputMode(window_handle_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      break;
    case MouseCursorMode::Disabled:
      glfwSetInputMode(window_handle_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      break;
    case MouseCursorMode::Unknown:
    default:
      break;
  }

  updated_input_state_.cursor_mode = MouseCursorMode::Unknown;
}
}  // namespace input
}  // namespace comet
