// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_GLFW_GLFW_WINDOW_H_
#define COMET_COMET_RENDERING_WINDOW_GLFW_GLFW_WINDOW_H_

#define GLFW_INCLUDE_NONE

#include "comet_precompile.h"

#include "GLFW/glfw3.h"

#include "comet/rendering/window/window.h"

namespace comet {
namespace rendering {
class GlfwWindow : public Window {
 public:
  GlfwWindow() = default;
  GlfwWindow(const WindowDescr& descr);
  GlfwWindow(const GlfwWindow&);
  GlfwWindow(GlfwWindow&&) noexcept;
  GlfwWindow& operator=(const GlfwWindow&);
  GlfwWindow& operator=(GlfwWindow&&) noexcept;
  virtual ~GlfwWindow() = default;

  virtual void Initialize() override;
  virtual void Destroy() override;
  virtual void SetGlfwHints();
  virtual void SetSize(unsigned int width, unsigned int height) override;

  virtual bool IsInitialized() const override;
  virtual const GLFWwindow* GetHandle() const noexcept;

 protected:
  inline static std::size_t window_count_ = 0;
  GLFWwindow* handle_ = nullptr;
  bool is_initialized_ = false;
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_GLFW_GLFW_WINDOW_H_