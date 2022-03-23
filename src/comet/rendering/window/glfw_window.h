// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_GLFW_WINDOW_H_
#define COMET_COMET_RENDERING_WINDOW_GLFW_WINDOW_H_

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "comet_precompile.h"
#include "window.h"

namespace comet {
namespace rendering {
class GlfwWindow : public Window {
 public:
  GlfwWindow(const std::string& = Window::kDefaultName_,
             unsigned int = kDefaultWidth_, unsigned int = kDefaultHeight_);
  GlfwWindow(const GlfwWindow&);
  GlfwWindow(GlfwWindow&&) = default;
  GlfwWindow& operator=(const GlfwWindow&);
  GlfwWindow& operator=(GlfwWindow&&) = default;
  virtual ~GlfwWindow() override;

  virtual void Initialize() override;
  virtual void Destroy() override;
  virtual void Update() override;
  virtual void SetSize(unsigned int, unsigned int) override;

  virtual GLFWwindow* GetGlfwWindow() const noexcept;
  virtual bool IsVSync() const noexcept;
  virtual void SetVSync(bool);

 protected:
  inline static std::size_t window_count_ = 0;
  GLFWwindow* window_ = nullptr;
  bool is_vsync_ = true;
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_GLFW_WINDOW_H_