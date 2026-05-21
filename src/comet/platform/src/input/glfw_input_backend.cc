// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/platform/input/glfw_input_backend.h"

#include "GLFW/glfw3.h"

#ifdef COMET_IMGUI
#include "imgui_impl_glfw.h"
#endif  // COMET_IMGUI

namespace comet {
namespace input {
void GlfwInputBackend::AttachWindow(GLFWwindow* window) {
  COMET_ASSERT(window != nullptr, "GlfwInputBackend::AttachWindow",
               "window is null");
  window_ = window;

#ifdef COMET_IMGUI
  glfwSetScrollCallback(window_, [](GLFWwindow* handle, f64 x, f64 y) {
    if (is_imgui_enabled_) {
      ImGui_ImplGlfw_ScrollCallback(handle, x, y);
    }
  });

  glfwSetCursorPosCallback(window_, [](GLFWwindow* handle, f64 x, f64 y) {
    if (is_imgui_enabled_) {
      ImGui_ImplGlfw_CursorPosCallback(handle, x, y);
    }
  });

  glfwSetKeyCallback(window_, [](GLFWwindow* handle, s32 key, s32 scan_code,
                                 s32 action, s32 mods) {
    if (is_imgui_enabled_) {
      ImGui_ImplGlfw_KeyCallback(handle, key, scan_code, action, mods);
    }
  });

  glfwSetMouseButtonCallback(
      window_, [](GLFWwindow* handle, s32 button, s32 action, s32 mods) {
        if (is_imgui_enabled_) {
          ImGui_ImplGlfw_MouseButtonCallback(handle, button, action, mods);
        }
      });

  glfwSetCharCallback(window_, [](GLFWwindow* handle, u32 code_point) {
    if (is_imgui_enabled_) {
      ImGui_ImplGlfw_CharCallback(handle, code_point);
    }
  });
#endif  // COMET_IMGUI
}

void GlfwInputBackend::DetachWindow() {
  if (window_ == nullptr) {
    return;
  }

  glfwSetScrollCallback(window_, nullptr);
  glfwSetCursorPosCallback(window_, nullptr);
  glfwSetKeyCallback(window_, nullptr);
  glfwSetMouseButtonCallback(window_, nullptr);
  glfwSetCharCallback(window_, nullptr);

  window_ = nullptr;
}

void GlfwInputBackend::Poll(InputSnapshot& snapshot) {
  COMET_ASSERT(window_ != nullptr, "GlfwInputBackend::Poll", "window is null");

  glfwPollEvents();

  for (usize i{0}; i < internal::kKeyCount; ++i) {
    const auto key{static_cast<s32>(i) + internal::kKeyBaseOffset};
    snapshot.keys[i] = glfwGetKey(window_, key) == GLFW_PRESS;
  }

  for (usize i{0}; i < internal::kMouseButtonCount; ++i) {
    snapshot.mouse_buttons[i] =
        glfwGetMouseButton(window_, static_cast<s32>(i)) == GLFW_PRESS;
  }

  f64 x{.0};
  f64 y{.0};
  glfwGetCursorPos(window_, &x, &y);
  snapshot.mouse_position = {static_cast<f32>(x), static_cast<f32>(y)};
}

void GlfwInputBackend::ApplyUserUpdate(const InputUserUpdate& update) {
  COMET_ASSERT(window_ != nullptr, "GlfwInputBackend::ApplyUserUpdate",
               "window is null");

  if (update.has_mouse_position) {
    glfwSetCursorPos(window_, update.mouse_position.x, update.mouse_position.y);
  }

  switch (update.cursor_mode) {
    case MouseCursorMode::Normal:
      glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      break;
    case MouseCursorMode::Disabled:
      glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      break;
    case MouseCursorMode::Unknown:
      break;
  }
}

#ifdef COMET_IMGUI
void GlfwInputBackend::EnableImGui() { is_imgui_enabled_ = true; }
#endif  // COMET_IMGUI
}  // namespace input
}  // namespace comet