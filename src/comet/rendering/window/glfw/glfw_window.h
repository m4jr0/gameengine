// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_GLFW_GLFW_WINDOW_H_
#define COMET_COMET_RENDERING_WINDOW_GLFW_GLFW_WINDOW_H_

#include "comet_precompile.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "comet/rendering/rendering_common.h"
#include "comet/rendering/window/window.h"

namespace comet {
namespace rendering {
class GlfwWindow : public Window {
 public:
  GlfwWindow() = delete;
  explicit GlfwWindow(const WindowDescr& descr);
  GlfwWindow(const GlfwWindow&);
  GlfwWindow(GlfwWindow&&) noexcept;
  GlfwWindow& operator=(const GlfwWindow&);
  GlfwWindow& operator=(GlfwWindow&&) noexcept;
  virtual ~GlfwWindow() = default;

  virtual void Initialize() override;
  virtual void Destroy() override;
  virtual void Update() override;
  virtual void SetGlfwHints();
  virtual void SetSize(WindowSize width, WindowSize height) override;
  void SetUpResizeEvent(WindowSize width, WindowSize height);

  virtual GLFWwindow* GetHandle() noexcept;
  operator GLFWwindow*() noexcept;

 protected:
  inline static uindex window_count_{0};
  GLFWwindow* handle_{nullptr};

 private:
  bool is_resize_event_{false};
  WindowSize new_width_{0};
  WindowSize new_height_{0};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_GLFW_GLFW_WINDOW_H_