// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_PLATFORM_WINDOW_GLFW_EMPTY_EMPTY_GLFW_WINDOW_H_
#define COMET_PLATFORM_WINDOW_GLFW_EMPTY_EMPTY_GLFW_WINDOW_H_

#include "comet/core/essentials.h"
#include "comet/rendering/window/glfw/glfw_window.h"

namespace comet {
namespace platform {
class EmptyGlfwWindow : public GlfwWindow {
 public:
  EmptyGlfwWindow() = delete;
  explicit EmptyGlfwWindow(WindowDescr& descr);
  EmptyGlfwWindow(const EmptyGlfwWindow&);
  EmptyGlfwWindow(EmptyGlfwWindow&&) noexcept;
  EmptyGlfwWindow& operator=(const EmptyGlfwWindow&);
  EmptyGlfwWindow& operator=(EmptyGlfwWindow&&) noexcept;
  ~EmptyGlfwWindow() override = default;
};
}  // namespace platform
}  // namespace comet

#endif  // COMET_PLATFORM_WINDOW_GLFW_EMPTY_EMPTY_GLFW_WINDOW_H_