// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "empty_glfw_window.h"

namespace comet {
namespace rendering {
namespace empty {
EmptyGlfwWindow::EmptyGlfwWindow(WindowDescr& descr) : GlfwWindow{descr} {}

EmptyGlfwWindow::EmptyGlfwWindow(const EmptyGlfwWindow& other)
    : GlfwWindow{other} {}

EmptyGlfwWindow::EmptyGlfwWindow(EmptyGlfwWindow&& other) noexcept
    : GlfwWindow{std::move(other)} {}

EmptyGlfwWindow& EmptyGlfwWindow::operator=(const EmptyGlfwWindow& other) {
  if (this == &other) {
    return *this;
  }

  GlfwWindow::operator=(other);
  return *this;
}

EmptyGlfwWindow& EmptyGlfwWindow::operator=(EmptyGlfwWindow&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  GlfwWindow::operator=(std::move(other));
  return *this;
}
}  // namespace empty
}  // namespace rendering
}  // namespace comet
