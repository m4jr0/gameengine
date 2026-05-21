// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_PLATFORM_INPUT_GLFW_INPUT_BACKEND_H_
#define COMET_PLATFORM_INPUT_GLFW_INPUT_BACKEND_H_

#include "comet/core/essentials.h"
#include "comet/runtime/input/input_backend.h"

struct GLFWwindow;

namespace comet {
namespace input {
class GlfwInputBackend final : public InputBackend {
 public:
  GlfwInputBackend() = default;
  GlfwInputBackend(const GlfwInputBackend&) = delete;
  GlfwInputBackend(GlfwInputBackend&&) = delete;
  GlfwInputBackend& operator=(const GlfwInputBackend&) = delete;
  GlfwInputBackend& operator=(GlfwInputBackend&&) = delete;
  ~GlfwInputBackend() override = default;

  void AttachWindow(GLFWwindow* window);
  void DetachWindow();

  void Poll(InputSnapshot& snapshot) override;
  void ApplyUserUpdate(const InputUserUpdate& update) override;

#ifdef COMET_IMGUI
  static void EnableImGui();
#endif  // COMET_IMGUI

 private:
  GLFWwindow* window_{nullptr};

#ifdef COMET_IMGUI
  static inline bool is_imgui_enabled_{false};
#endif  // COMET_IMGUI
};
}  // namespace input
}  // namespace comet

#endif  // COMET_PLATFORM_INPUT_GLFW_INPUT_BACKEND_H_