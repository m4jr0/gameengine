// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_GLFW_EMPTY_EMPTY_GLFW_WINDOW_H_
#define COMET_COMET_RENDERING_WINDOW_GLFW_EMPTY_EMPTY_GLFW_WINDOW_H_

#include "comet/core/essentials.h"
#include "comet/rendering/window/glfw/glfw_window.h"

namespace comet {
namespace rendering {
namespace empty {
class EmptyGlfwWindow : public GlfwWindow {
 public:
  EmptyGlfwWindow() = delete;
  explicit EmptyGlfwWindow(WindowDescr& descr);
  EmptyGlfwWindow(const EmptyGlfwWindow&);
  EmptyGlfwWindow(EmptyGlfwWindow&&) noexcept;
  EmptyGlfwWindow& operator=(const EmptyGlfwWindow&);
  EmptyGlfwWindow& operator=(EmptyGlfwWindow&&) noexcept;
  virtual ~EmptyGlfwWindow() = default;
};
}  // namespace empty
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_GLFW_EMPTY_EMPTY_GLFW_WINDOW_H_