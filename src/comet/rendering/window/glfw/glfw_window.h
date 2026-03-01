// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_GLFW_GLFW_WINDOW_H_
#define COMET_COMET_RENDERING_WINDOW_GLFW_GLFW_WINDOW_H_

#include "comet/core/essentials.h"

// External. ///////////////////////////////////////////////////////////////////

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
////////////////////////////////////////////////////////////////////////////////

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

  virtual GLFWwindow* GetHandle() noexcept;
  operator GLFWwindow*() noexcept;

 protected:
  inline static usize window_count_{0};
  GLFWwindow* handle_{nullptr};

 private:
  void UpdateSize();
  void DumpPlatform() const;

  bool is_resize_{false};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_GLFW_GLFW_WINDOW_H_