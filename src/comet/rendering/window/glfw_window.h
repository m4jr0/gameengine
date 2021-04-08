// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_GLFW_WINDOW_H_
#define COMET_COMET_RENDERING_WINDOW_GLFW_WINDOW_H_

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "comet_precompile.h"
#include "window.h"

namespace comet {
class GlfwWindow : public Window {
 public:
  GlfwWindow(const std::string & = Window::kDefaultName_,
             unsigned int = kDefaultWidth_, unsigned int = kDefaultHeight_);
  GlfwWindow(const GlfwWindow &) = delete;
  GlfwWindow(GlfwWindow &&) = delete;
  GlfwWindow &operator=(const GlfwWindow &) = delete;
  GlfwWindow &operator=(GlfwWindow &&) = delete;
  virtual ~GlfwWindow() override;

  virtual void Initialize() override;
  virtual void Destroy() override;
  virtual void Update() override;
  virtual void SetSize(unsigned int, unsigned int) override;

  virtual GLFWwindow *glfw_window() const noexcept;
  virtual bool is_vsync() const noexcept;
  virtual void is_vsync(bool);

 protected:
  inline static std::size_t window_count_ = 0;
  GLFWwindow *window_ = nullptr;
  bool is_vsync_ = true;
};
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_GLFW_WINDOW_H_